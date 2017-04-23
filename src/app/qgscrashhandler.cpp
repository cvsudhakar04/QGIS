/***************************************************************************
  qgscrashhandler.cpp - QgsCrashHandler

 ---------------------
 begin                : 23.4.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscrashhandler.h"

#include <QProcess>
#include <QDir>

#include "qgsproject.h"
#include "qgscrashdialog.h"
#include "qgscrashreport.h"

#ifdef Q_OS_WIN
LONG WINAPI QgsCrashHandler::handle( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
  HANDLE process = GetCurrentProcess();
  // TOOD Pull symbols from symbol server.
  // TOOD Move this logic to generic stack trace class to handle each
  // platform.
  SymInitialize( process, NULL, TRUE );

  // StackWalk64() may modify context record passed to it, so we will
  // use a copy.
  CONTEXT context_record = *ExceptionInfo->ContextRecord;
  // Initialize stack walking.
  STACKFRAME64 stack_frame;
  memset( &stack_frame, 0, sizeof( stack_frame ) );
#if defined(_WIN64)
  int machine_type = IMAGE_FILE_MACHINE_AMD64;
  stack_frame.AddrPC.Offset = context_record.Rip;
  stack_frame.AddrFrame.Offset = context_record.Rbp;
  stack_frame.AddrStack.Offset = context_record.Rsp;
#else
  int machine_type = IMAGE_FILE_MACHINE_I386;
  stack_frame.AddrPC.Offset = context_record.Eip;
  stack_frame.AddrFrame.Offset = context_record.Ebp;
  stack_frame.AddrStack.Offset = context_record.Esp;
#endif
  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Mode = AddrModeFlat;

  SYMBOL_INFO *symbol = ( SYMBOL_INFO * ) qgsMalloc( sizeof( SYMBOL_INFO ) + MAX_SYM_NAME );
  symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
  symbol->MaxNameLen = MAX_SYM_NAME;

  IMAGEHLP_LINE *line = ( IMAGEHLP_LINE * ) qgsMalloc( sizeof( IMAGEHLP_LINE ) );
  line->SizeOfStruct = sizeof( IMAGEHLP_LINE );

  IMAGEHLP_MODULE *module = ( IMAGEHLP_MODULE * ) qgsMalloc( sizeof( IMAGEHLP_MODULE ) );
  module->SizeOfStruct = sizeof( IMAGEHLP_MODULE );

  QList<QgsCrashReport::StackLine> stack;
  while ( StackWalk64( machine_type,
                       GetCurrentProcess(),
                       GetCurrentThread(),
                       &stack_frame,
                       &context_record,
                       NULL,
                       &SymFunctionTableAccess64,
                       &SymGetModuleBase64,
                       NULL ) )
  {

    DWORD64 displacement = 0;

    if ( SymFromAddr( process, ( DWORD64 )stack_frame.AddrPC.Offset, &displacement, symbol ) )
    {
      DWORD dwDisplacement;
      QString fileName;
      QString lineNumber;
      QString moduleName;
      if ( SymGetLineFromAddr( process, ( DWORD )( stack_frame.AddrPC.Offset ), &dwDisplacement, line ) )
      {
        fileName = QString( line->FileName );
        lineNumber = QString::number( line->LineNumber );
      }
      else
      {
        fileName = "(unknown file)";
        lineNumber = "(unknown line)";
      }
      if ( SymGetModuleInfo( process, ( DWORD )( stack_frame.AddrPC.Offset ), module ) )
      {
        moduleName = QString( module->ModuleName );
      }
      else
      {
        moduleName = "(unknown module)";
      }
      QgsCrashReport::StackLine stackline;
      stackline.moduleName = moduleName;
      stackline.fileName = fileName;
      stackline.lineNumber = lineNumber;
      stackline.symbolName = QString( symbol->Name );
      stack.append( stackline );
    }
  }

  qgsFree( symbol );
  qgsFree( line );
  qgsFree( module );

  showCrashDialog( stack );

  return EXCEPTION_EXECUTE_HANDLER;
}

void QgsCrashHandler::showCrashDialog( const QList<QgsCrashReport::StackLine> &stack )
{

  QgsCrashDialog dlg( QApplication::activeWindow() );
  QgsCrashReport report;
  report.setStackTrace( stack );
  dlg.setBugReport( report.toString() );
  if ( dlg.exec() )
  {
    restartApplication();
  }
}

void QgsCrashHandler::restartApplication()
{
  QStringList arguments;
  arguments = QCoreApplication::arguments();
  QString path = arguments.at( 0 );
  arguments.removeFirst();
  arguments << QgsProject::instance()->fileName();
  QProcess::startDetached( path, arguments, QDir::toNativeSeparators( QCoreApplication::applicationDirPath() ) );

}
#endif
