/* ******************************************************************** **
** @@ DBX DBF-2-TXT dumper
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Note   :
** ******************************************************************** */

/* ******************************************************************** **
**                uses pre-compiled headers
** ******************************************************************** */

#include "stdafx.h"

#include <stdio.h>
#include <limits.h>
#include <time.h>

#include "..\shared\xlat_tables.h"
#include "..\shared\xlat.h"
#include "..\shared\text.h"
#include "..\shared\file.h"
#include "..\shared\file_walker.h"
#include "..\shared\mmf.h"
#include "..\shared\timestamp.h"
#include "..\shared\vector.h"
#include "..\shared\vector_sorted.h"
#include "..\shared\db_dbx.h"

#ifdef NDEBUG
#pragma optimize("gsy",on)
#pragma comment(linker,"/FILEALIGN:512 /MERGE:.rdata=.text /MERGE:.data=.text /SECTION:.text,EWR /IGNORE:4078")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* ******************************************************************** **
** @@                   internal defines
** ******************************************************************** */

#ifndef QWORD
typedef unsigned __int64   QWORD;
#endif

#define  MAX_RECORD_SIZE                        (128)

/* ******************************************************************** **
** @@                   internal prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@                   external global variables
** ******************************************************************** */

extern DWORD   dwKeepError = 0;

/* ******************************************************************** **
** @@                   static global variables
** ******************************************************************** */

static DWORD               _dwGranulation = 3; // 2 Power: 0, 2, 3, 4

static char                _pRecord[MAX_RECORD_SIZE];

static DBX_TABLE_INFO      _InfoSrc;
static DBX_TABLE_INFO      _InfoDst;
                              
/* ******************************************************************** **
** @@                   real code
** ******************************************************************** */

/* ******************************************************************** **
** @@ Dump()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void Dump
(
   const char* const       pszFile,
   const char* const       pszTxt
)
{
   ASSERT(pszFile);
   ASSERT(pszTxt);

   char     pszDBFName [_MAX_PATH];
   char     pszDBVName [_MAX_PATH];
   char     pszTxtName [_MAX_PATH];
   char     pszDrive   [_MAX_DRIVE];
   char     pszDir     [_MAX_DIR];
   char     pszFName   [_MAX_FNAME];
   char     pszExt     [_MAX_EXT];

   _splitpath(pszFile,    pszDrive,pszDir,pszFName,pszExt);
   _makepath( pszDBFName, pszDrive,pszDir,pszFName,"dbf");
   _makepath( pszDBVName, pszDrive,pszDir,pszFName,"dbv");
   _makepath( pszTxtName, pszDrive,pszDir,pszFName,"txt");

   strcpy((char*)&_InfoSrc._pszName,pszDBFName);

   DBX*       pDBX = new DBX;

   DBX_TABLE*     pTable = pDBX->OpenTable(pszFName,pszDBFName,NULL,DBX_OM_READ_ONLY,DBX_OM_READ_ONLY);
   
   if (!pTable)
   {
      // Error !
      ASSERT(0);
      delete pDBX;
      pDBX = NULL;
      return;
   }

   MMF*     pMF = new MMF;

   ASSERT(pMF);

   pMF->OpenReadOnly(pszDBVName);

   if (!pMF)
   {
      // Error !
      ASSERT(0);
      delete pTable;
      pTable = NULL;
      delete pDBX;
      pDBX = NULL;
      return;
   }

   BYTE*       pBuf   = pMF->Buffer();
   DWORD       dwSize = pMF->Size();

   ASSERT(pBuf);
   ASSERT(dwSize);

   DWORD    dwRecCnt = pTable->GetRecCnt();

   ASSERT(dwRecCnt);

   FILE*    pOut = fopen(pszTxtName,"wt");

   if (pOut)
   {
      for (DWORD ii = 1; ii <= dwRecCnt; ++ii)
      {
         const BYTE* const    pRecord = pTable->Go(ii);

         ASSERT(pRecord);

         DBX_COLUMN*    pIndex = pTable->GetColumn("INDEX");
         DBX_COLUMN*    pGuid  = pTable->GetColumn("GUID");
         DBX_COLUMN*    pText  = pTable->GetColumn("TEXT");

         ASSERT(pIndex);
         ASSERT(pGuid);
         ASSERT(pText);

         DWORD    dwText = *(DWORD*)pText->Get(pRecord);

         fprintf(pOut,"%s\n",pBuf + dwText + sizeof(DWORD));
      }
   }

   fclose(pOut);
   pOut = NULL;

   pDBX->CloseTable(pTable);

   delete pDBX;
   pDBX = NULL;

   pMF->Close();

   delete pMF;
   pMF = NULL;
}

/* ******************************************************************** **
** @@ ForEach()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void ForEach(const char* const pszFilename)
{
   char     pszTxtName[_MAX_PATH];
   char     pszDrive  [_MAX_DRIVE];
   char     pszDir    [_MAX_DIR];
   char     pszFName  [_MAX_FNAME];
   char     pszExt    [_MAX_EXT];

   _splitpath(pszFilename,pszDrive,pszDir,pszFName,pszExt);
   _makepath( pszTxtName, pszDrive,pszDir,pszFName,"dbi");

   Dump(pszFilename,pszTxtName);
}

/* ******************************************************************** **
** @@ ShowHelp()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void ShowHelp()
{
   const char  pszCopyright[] = "-*-   DBF-2-TXT  1.0   *   Copyright (c) Gazlan, 2015   -*-";
   const char  pszDescript [] = "DBX DBF-2-TXT dumper";
   const char  pszE_Mail   [] = "complains_n_suggestions direct to gazlan@yandex.ru";

   printf("%s\n\n",pszCopyright);
   printf("%s\n\n",pszDescript);
   printf("Usage: dbf2txt.com wildcards\n\n");
   printf("%s\n",pszE_Mail);
}

/* ******************************************************************** **
** @@ main()
** @ Copyrt:
** @ Author:
** @ Modify:
** @ Update:
** @ Notes :
** ******************************************************************** */

int main(int argc,char** argv)
{
   if (argc != 2)
   {
      ShowHelp();
      return 0;
   }

   if (argc == 2 && ((!strcmp(argv[1],"?")) || (!strcmp(argv[1],"/?")) || (!strcmp(argv[1],"-?")) || (!stricmp(argv[1],"/h")) || (!stricmp(argv[1],"-h"))))
   {
      ShowHelp();
      return 0;
   }

//   _pOut = fopen("err_log.txt","wt");

   char     pszMask[MAX_PATH + 1];
   
   memset(pszMask,0,sizeof(pszMask));
   
   strncpy(pszMask,argv[1],MAX_PATH);
   pszMask[MAX_PATH] = 0; // Ensure ASCIIZ
   
   char     pszDrive[_MAX_DRIVE];
   char     pszDir  [_MAX_DIR];
   char     pszFName[_MAX_FNAME];
   char     pszExt  [_MAX_EXT];
   
   _splitpath(pszMask,pszDrive,pszDir,pszFName,pszExt);
   
   char     pszSrchMask[MAX_PATH + 1];
   char     pszSrchPath[MAX_PATH + 1];
   
   strcpy(pszSrchMask,pszFName);
   strcat(pszSrchMask,pszExt);
   
   Walker      Visitor;

   Visitor.Init(ForEach,pszSrchMask,false);

   strcpy(pszSrchPath,pszDrive);
   strcat(pszSrchPath,pszDir);

   Visitor.Run(*pszSrchPath  ?  pszSrchPath  :  ".");

   return 0;
}

/* ******************************************************************** **
**                That's All
** ******************************************************************** */
