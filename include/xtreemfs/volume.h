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


#ifndef _XTREEMFS_VOLUME_H_
#define _XTREEMFS_VOLUME_H_

#include "xtreemfs/dir_proxy.h"
#include "xtreemfs/mrc_proxy.h"
#include "xtreemfs/osd_proxies.h"


namespace xtreemfs
{
  class File;
  class OpenFileTable;
  class Options;
  class Stat;
  class StatCache;
  using org::xtreemfs::interfaces::FileCredentials;
  using org::xtreemfs::interfaces::VivaldiCoordinates;
  using org::xtreemfs::interfaces::XCap;

  using yield::concurrency::StageGroup;
  using yield::platform::Mutex;
  using yield::platform::Path;


  class Volume : public yield::platform::Volume
  {
  public:
    static uint32_t ERROR_CODE_DEFAULT; // The error code for internal exceptions

    const static uint32_t FLAG_WRITE_BACK_DATA_CACHE = 1;
    const static uint32_t FLAG_WRITE_BACK_FILE_SIZE_CACHE = 2;
    const static uint32_t FLAG_WRITE_BACK_STAT_CACHE = 4;
    const static uint32_t FLAG_WRITE_THROUGH_DATA_CACHE = 8;
    const static uint32_t FLAG_WRITE_THROUGH_FILE_SIZE_CACHE = 16;
    const static uint32_t FLAG_WRITE_THROUGH_STAT_CACHE = 32;
    const static uint32_t FLAGS_DEFAULT = FLAG_WRITE_BACK_FILE_SIZE_CACHE;


    virtual ~Volume();

    void close( File& file );

    static Volume&
    create
    (
      const Options& options,
      uint32_t flags = FLAGS_DEFAULT,
      const Path& vivaldi_coordinates_file_path = Path()
    );

    static Volume&
    create
    (
      const URI& dir_uri,
      const string& name_utf8,
      Log* error_log = NULL,
      uint32_t flags = FLAGS_DEFAULT,
#ifdef YIELD_PLATFORM_HAVE_OPENSSL
      SSLContext* proxy_ssl_context = NULL,
#endif
      Log* trace_log = NULL,
      const Path& vivaldi_coordinates_file_path = Path()
    );

    // fsetattr is used for setting the file size
    void fsetattr
    (
      const Path& path,
      const Stat& stbuf,
      uint32_t to_set,
      const XCap& write_xcap
    );

    DIRProxy& get_dir_proxy() const { return dir_proxy; }
    uint32_t get_flags() const { return flags; }    
    MRCProxy& get_mrc_proxy() const { return mrc_proxy; }
    const string& get_name() const { return name_utf8; }
    OSDProxies& get_osd_proxies() const { return osd_proxies; }
    Log* get_trace_log() const { return trace_log; }
    UserDatabase& get_user_database() const;
    const string& get_uuid() const { return uuid; }
    VivaldiCoordinates get_vivaldi_coordinates() const;
    void metadatasync( const Path& path, const XCap& write_xcap );    
    void set_errno( const char* operation_name, Exception& exception );
    void set_errno( const char* operation_name, std::exception& exception );

    // yidl::runtime::Object
    Volume& inc_ref() { return Object::inc_ref( *this ); }

    // yield::platform::Volume
    YIELD_PLATFORM_VOLUME_PROTOTYPES;

  private:
    Volume
    (
      DIRProxy& dir_proxy,
      Log* error_log,
      uint32_t flags,
      MRCProxy& mrc_proxy,
      const string& name_utf8,
      OSDProxies& osd_proxies,
      StageGroup& stage_group,
      Log* trace_log,
      UserDatabase& user_database,
      const Path& vivaldi_coordinates_file_path
    );

    void osd_unlink( const FileCredentials& file_credentials );

  private:
    DIRProxy& dir_proxy;
    Log* error_log;
    class FileState;
    typedef map<string, FileState*> FileStateMap;
    FileStateMap file_state_map;
    Mutex file_state_map_lock;
    uint32_t flags;
    MRCProxy& mrc_proxy;
    string name_utf8;
    OSDProxies& osd_proxies;
    StageGroup& stage_group;
    string uuid;
    StatCache* stat_cache;
    Log* trace_log;
    UserDatabase& user_database;
    Path vivaldi_coordinates_file_path;
  };
};

#endif
