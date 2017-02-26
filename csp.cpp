// -----------------------------------------------------------------
// Copyright (C) 2017  Gabriele Bonacini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
// -----------------------------------------------------------------

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <libelf.h>
#include <gelf.h>

using namespace std;

enum class Verbosity : int {NONE=0, STD=1, VERB=2, DEBUG=3};
const int   ERRCODE      =   3;

class ElfPrsExp final{
        public:
                     ElfPrsExp(string&  errString);
                     ElfPrsExp(string&& errString);
              string what(void)                                  const  noexcept(true);
           private:
              string errorMessage;
};

ElfPrsExp::ElfPrsExp(string& errString){
        errorMessage            = errString;
}

ElfPrsExp::ElfPrsExp(string&& errString){
        errorMessage            = move(errString);
}

string ElfPrsExp::what() const noexcept(true){
        return errorMessage;
}

class ElfPrs{
    public:
             ElfPrs(string fname, Verbosity dbg=Verbosity::NONE);
             ~ElfPrs(void);
       bool  checkSection(const string  sectionName,
                          const string  symbolName)                   noexcept(false);
       template <typename ...Ts>
       void pdb(Verbosity v, Ts... ts)                          const noexcept(true); 
    private:
       int         fd;
       string      filename;
       Verbosity   elfDebug;
       size_t      shstrndx;
       Elf         *elf;
       Elf_Scn     *scn;
       GElf_Shdr   gshdr;
};

ElfPrs::ElfPrs(string fname, Verbosity  dbg) : filename{fname}, elfDebug{dbg}, elf{nullptr}, scn{nullptr}{
    if(elf_version(EV_CURRENT) == EV_NONE)
          throw ElfPrsExp(string("ELF library init error: ") + elf_errmsg(-1));

    fd     = open(filename.c_str(), O_RDONLY, 0);
    if(fd < 0) 
          throw ElfPrsExp(string("ELF library init error opening file: " + filename));

    elf    = elf_begin(fd, ELF_C_READ, nullptr);
    if(elf == nullptr) 
          throw ElfPrsExp(string("ELF library init error - elf_begin(): ") + elf_errmsg(-1));

    if(elf_kind(elf) != ELF_K_ELF)
          throw ElfPrsExp(string("ELF library init error - invalid file type: ") + filename);

    if(elf_getshdrstrndx(elf, &shstrndx) != 0)
          throw ElfPrsExp(string("ELF library init error - elf_getshdrstrndx(): ") + elf_errmsg(-1));
}

ElfPrs::~ElfPrs(void){
     elf_end(elf);
     close(fd);
}

bool ElfPrs::checkSection(const string  sectionName, const string  symbolName) noexcept(false){
    bool       ret               = false;
    char       *section_name     = nullptr;

    pdb(Verbosity::STD, "\nProcessing file: %s ", filename.c_str());
    while((scn = elf_nextscn(elf, scn))!= nullptr){
        if(gelf_getshdr(scn, &gshdr) != &gshdr)
             throw ElfPrsExp(string("Binary checking error - getshdr(): ") + elf_errmsg(-1));

        section_name = elf_strptr(elf, shstrndx, gshdr.sh_name);
        if(section_name == nullptr)
             throw ElfPrsExp(string("Binary checking error - elf_strptr(): ") + elf_errmsg(-1));

        pdb(Verbosity::DEBUG, "Section: %s - %-4.4jd\n", 
                section_name, static_cast<uintmax_t>(elf_ndxscn(scn)));

        if(!strcmp(section_name, sectionName.c_str())) {
            ret = (symbolName.empty() ? true : false);
            pdb(Verbosity::STD, "- Found section: %s", sectionName.c_str());
            Elf_Data *edata         = elf_getdata(scn, nullptr);
            int symbol_count        = gshdr.sh_size / gshdr.sh_entsize;
  
            for(int i = 0; i < symbol_count; i++) {
                GElf_Sym sym;                   
                gelf_getsym(edata, i, &sym);
   
                char *thisSymb = elf_strptr(elf, gshdr.sh_link, sym.st_name);
   
                pdb(Verbosity::DEBUG, "%s - item: %d - %08x %08d\n", thisSymb, i, 
                           static_cast<unsigned int>(sym.st_value), 
                           static_cast<int>(sym.st_size));
                if(!symbolName.empty() && !strcmp(symbolName.c_str(), thisSymb)){
                    pdb(Verbosity::STD, " - found symbol: %s", symbolName.c_str());
                    ret = true;
                }
          }
          break;
        }
    }

    if(ret) pdb(Verbosity::STD, "\n");
    return ret;
}

template <typename ...Ts>
void ElfPrs::pdb(Verbosity v, Ts... ts)  const noexcept(true){
  if(v <= elfDebug) fprintf(stderr, ts...);
}

[[ noreturn ]] void printHelp(const char* cmd){
              cerr << "\nusage: " << cmd << " [-v | -vv | -vvv] [-h] file-name" << endl;
              cerr << " -v, -vv, -vvv  verbosity " << endl; 
              cerr << " -h             this help message " << endl; 
              cerr << "Return codes:\n 0 protected\n 1 not protected\n" 
                      " 3 error (wrong file type, etc)." << endl;
              exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){
     const char    flags[]   = "hv";
     int           c,
                   ret       = ERRCODE,
                   v         = 0;
     Verbosity     verbose   = Verbosity::NONE;

     opterr = 0;
     while ((c = getopt(argc, argv, flags)) != -1){
        switch (c){
           case 'v':
              v++;
              switch(v){
                  case static_cast<int>(Verbosity::STD): 
                      verbose = Verbosity::STD;
                  break;
                  case static_cast<int>(Verbosity::VERB): 
                      verbose = Verbosity::VERB;
                  break;
                  case static_cast<int>(Verbosity::DEBUG): 
                      verbose = Verbosity::DEBUG;
              } 
           break;
           case 'h':
           default:
              printHelp(argv[0]);
        }
     }

     if(optind == argc) printHelp(argv[0]);

     try{
          ElfPrs elfprs(argv[optind], verbose);
          ret = elfprs.checkSection(".dynsym",  "__stack_chk_fail") ? 0 : 1;

     }catch(ElfPrsExp& ex){
          cerr << "\nElfPrs exception: " <<  ex.what() << endl;

     }catch(...){
          cerr << "\nUnhandled expression!" << endl;
     }

     return ret;
}
