#include <iostream>
#include <string> 
#include <fstream>

#include <stdlib.h>
#include<stdarg.h>

#include "elf.h"

using namespace std;

ofstream outFile;

void Say( char * pszFormat, ...) 
{ 
    va_list args; 
    va_start(args, pszFormat); 

    char szTmp[128];

    vsprintf(szTmp, pszFormat, args);

    cout << szTmp << endl; 

    va_end(args); 
}

bool ParseELF(unsigned char * pData) 
{ 
    Elf32_Ehdr * pELFHdr = (Elf32_Ehdr *)pData; 

    if (!(    (pELFHdr->e_ident[0] == 0x7F) && 
                (pELFHdr->e_ident[1] == 'E') && 
                (pELFHdr->e_ident[2] == 'L') && 
                (pELFHdr->e_ident[3] == 'F')    )) { 
        Say("? ELF ?????????????"); 
        return false; 
    } 

    Say("\r\n\r\n------------ ELF Header ---------------\r\n"); 
    Say("e_ident      %10Xh, ", pELFHdr->e_ident[0]); 
    Say("\"%c", pELFHdr->e_ident[1]); 
    Say("%c", pELFHdr->e_ident[2]); 
    Say("%c\", ", pELFHdr->e_ident[3]); 
    Say("%xh, ", pELFHdr->e_ident[4]); 
    Say("%xh, ", pELFHdr->e_ident[5]); 
    Say("%xh, ...\r\n", pELFHdr->e_ident[6]); 
    Say("e_type       %10d    %s\r\n", pELFHdr->e_type, pELFHdr->e_type > 4 ? "Processor-specific" : sz_desc_e_type[pELFHdr->e_type]); 
    Say("e_machine    %10d    %s\r\n", pELFHdr->e_machine, sz_desc_e_machine[pELFHdr->e_machine]); 
    Say("e_version    %10d\r\n", pELFHdr->e_version); 
    Say("e_entry      %10X H    %s\r\n", pELFHdr->e_entry, sz_desc_e_entry); 
    Say("e_phoff      %10X H\r\n", pELFHdr->e_phoff); 
    Say("e_shoff      %10X H\r\n", pELFHdr->e_shoff); 
    Say("e_flags      %10d\r\n", pELFHdr->e_flags); 
    Say("e_ehsize     %10X H\r\n", pELFHdr->e_ehsize); 
    Say("e_phentsize  %10X H\r\n", pELFHdr->e_phentsize); 
    Say("e_phnum      %10d\r\n", pELFHdr->e_phnum); 
    Say("e_shentsize  %10X H\r\n", pELFHdr->e_shentsize); 
    Say("e_shnum      %10X H\r\n", pELFHdr->e_shnum); 
    Say("e_shstrndx   %10X H\r\n", pELFHdr->e_shstrndx); 

    Elf32_Phdr * pPHdr = (Elf32_Phdr *)(pData + pELFHdr->e_phoff); 
    for(int i=0; i < pELFHdr->e_phnum; i++,pPHdr++){ 
        Say("\r\n------------- Program Header %d -------------\r\n\r\n", i); 
        Say("p_type       %10X H    %s\r\n", pPHdr->p_type, pPHdr->p_type > 6 ? "PT_??PROC" : sz_desc_p_type[pPHdr->p_type]); 
        Say("p_offset     %10X H\r\n", pPHdr->p_offset); 
        Say("p_vaddr      %10X H\r\n", pPHdr->p_vaddr); 
        Say("p_paddr      %10X H\r\n", pPHdr->p_paddr); 
        Say("p_filesz     %10X H\r\n", pPHdr->p_filesz); 
        Say("p_memsz      %10X H\r\n", pPHdr->p_memsz); 
        Say("p_flags      %10X H\r\n", pPHdr->p_flags); 
        Say("p_align      %10X H\r\n", pPHdr->p_align); 
    } 

    char * pStrTable;    // ?????? section ??? 
    Elf32_Shdr * pSHdrStrTab = (Elf32_Shdr *)(pData + pELFHdr->e_shoff) + pELFHdr->e_shstrndx; 
    pStrTable = (char *)(pData + pSHdrStrTab->sh_offset); 

    Elf32_Shdr * pSHdr = (Elf32_Shdr *)(pData + pELFHdr->e_shoff); 
    for(int i=0; i < pELFHdr->e_shnum; i++,pSHdr++){ 
        Say("\r\n------------- Section Header %d -------------\r\n\r\n", i); 
        Say("sh_name      %10X H    %s\r\n", pSHdr->sh_name, pStrTable + pSHdr->sh_name); 
        Say("sh_type      %10X H    %s\r\n", pSHdr->sh_type, pSHdr->sh_type > 11 ? "??" : sz_desc_sh_type[pSHdr->sh_type]); 
        Say("sh_flags     %10X H    %s\r\n", pSHdr->sh_flags, pSHdr->sh_flags > 7 ? "SHF_MASKPROC" : sz_desc_sh_flags[pSHdr->sh_flags]); 
        Say("sh_addr      %10X H    %s\r\n", pSHdr->sh_addr, sz_desc_sh_addr); 
        Say("sh_offset    %10X H    %s\r\n", pSHdr->sh_offset, sz_desc_sh_offset); 
        Say("sh_size      %10X H\r\n", pSHdr->sh_size); 
        Say("sh_link      %10X H\r\n", pSHdr->sh_link); 
        Say("sh_info      %10X H\r\n", pSHdr->sh_info); 
        Say("sh_addralign %10X H\r\n", pSHdr->sh_addralign); 
        Say("sh_entsize   %10X H\r\n", pSHdr->sh_entsize); 
    } 

    Say("\r\n---------------------------------------\r\n\r\n"); 

    return true; 
}

void Usage()
{
    cout << "Usage: elf filename" << endl;
    return;
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        Usage();
        return 1;
    }

    ifstream inFile;

    inFile.open(argv[1], ios::binary);

    if (!inFile.is_open())
    {
        cout << "open the file failed" << endl;
        return 1;
    }

    //???????
    inFile.seekg(0, ios::end);

    int length = inFile.tellg();

    cout << "the file length is: " << length / 1024 << "." << length % 1024 << "KB" << endl;

    char* pBuf = (char*)malloc(length);

    //????????????
    inFile.seekg(0, ios::beg);

    inFile.read(pBuf, length);

    cout << "read count: " << inFile.gcount() << endl;

    outFile.open("result.txt");

    if(ParseELF((unsigned char*)pBuf))
        cout << "parse the file successfully, the result is saved into the file-result.txt !" << endl;

    free(pBuf);
    inFile.close();
    outFile.close();

    return 0;
}

