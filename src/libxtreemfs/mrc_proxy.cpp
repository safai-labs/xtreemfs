// Copyright (c) 2010 NEC HPC Europe
// All rights reserved
// 
// This source file is part of the XtreemFS project.
// It is licensed under the New BSD license:
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// * Neither the name of the XtreemFS project nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL NEC HPC Europe BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "xtreemfs/mrc_proxy.h"
#include "xtreemfs/options.h"
#include "user_database.h"
using namespace xtreemfs;



MRCProxy::MRCProxy
(
  EventHandler& request_handler,
  const char* password,
  UserDatabase* user_database
) : MRCInterfaceProxy( request_handler ),
    password( password )
{
  if ( user_database != NULL )
    this->user_database = Object::inc_ref( user_database );
  else
    this->user_database = new UserDatabase;
}

MRCProxy::~MRCProxy()
{
  UserDatabase::dec_ref( *user_database );
}

MRCProxy& 
MRCProxy::create
( 
  const URI& absolute_uri,
  const Options& options,
  const char* password
)
{
  return create
         ( 
           absolute_uri,
           options.get_error_log(),
           password,
#ifdef YIELD_PLATFORM_HAVE_OPENSSL
           options.get_ssl_context(),
#endif
           options.get_trace_log()
         );
}

MRCProxy&
MRCProxy::create
(
  const URI& absolute_uri,
  Log* error_log,
  const char* password,
#ifdef YIELD_PLATFORM_HAVE_OPENSSL
  SSLContext* ssl_context,
#endif
  Log* trace_log,
  UserDatabase* user_database
)
{
  return *new MRCProxy
              (
                createONCRPCClient
                (
                  absolute_uri,
                  *new org::xtreemfs::interfaces::MRCInterfaceMessageFactory,
                  ONC_RPC_PORT_DEFAULT,
                  0x20000000 + TAG,
                  TAG,
                  error_log,
#ifdef YIELD_PLATFORM_HAVE_OPENSSL
                  ssl_context,
#endif
                  trace_log
                ),
                password,  
                user_database
              );
}

void MRCProxy::handle( Request& request )
{
  if ( request.get_credentials() == NULL )
  {
    UserCredentials* user_credentials
      = user_database->getCurrentUserCredentials();

    if ( user_credentials != NULL )
      user_credentials->set_password( password );

    request.set_credentials( user_credentials );
  }

  MRCInterfaceProxy::handle( request );
}
