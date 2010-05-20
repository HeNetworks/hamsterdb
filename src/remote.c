/**
 * Copyright (C) 2005-2008 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 *
 */

#include "config.h"

#include <string.h>

#include "db.h"
#include "env.h"
#include "mem.h"
#include "messages.pb-c.h"

#if HAM_ENABLE_REMOTE

#include <curl/curl.h>
#include <curl/easy.h>

typedef struct curl_buffer_t
{
    ham_size_t packed_size;
    ham_u8_t *packed_data;
    ham_size_t offset;
    Ham__Wrapper *wrapper;
} curl_buffer_t;

static size_t
__writefunc(void *buffer, size_t size, size_t nmemb, void *ptr)
{
    curl_buffer_t *buf=(curl_buffer_t *)ptr;

    buf->wrapper=ham__wrapper__unpack(0, nmemb*size, (ham_u8_t *)buffer);
    if (!buf->wrapper)
        return 0;
    return size*nmemb;
}

static size_t
__readfunc(char *buffer, size_t size, size_t nmemb, void *ptr)
{
    curl_buffer_t *buf=(curl_buffer_t *)ptr;

    ham_assert(nmemb>=buf->packed_size, (""));
    if (buf->offset==buf->packed_size)
        return (0);

    ham_assert(buf->offset==0, (""));

    memcpy(buffer, buf->packed_data, buf->packed_size);
    buf->offset=buf->packed_size;
    return (buf->packed_size);
}

#define SETOPT(curl, opt, val)                                                \
                    if ((cc=curl_easy_setopt(curl, opt, val))) {              \
                        ham_trace(("curl_easy_setopt failed: %d/%s", cc,      \
                                    curl_easy_strerror(cc)));                 \
                        return (HAM_INTERNAL_ERROR);                          \
                    }

static ham_status_t
_perform_request(ham_env_t *env, CURL *handle, Ham__Wrapper *wrapper,
                Ham__Wrapper **reply)
{
    CURLcode cc;
    long response=0;
    char header[128];
    curl_buffer_t buf={0};
    struct curl_slist *slist=0;

    *reply=0;

    buf.packed_size=ham__wrapper__get_packed_size(wrapper);
    buf.packed_data=allocator_alloc(env_get_allocator(env), buf.packed_size);
    if (!buf.packed_data)
        return (HAM_OUT_OF_MEMORY);
    ham__wrapper__pack(wrapper, buf.packed_data);

    sprintf(header, "Content-Length: %u", buf.packed_size);
    slist=curl_slist_append(slist, header);
    slist=curl_slist_append(slist, "Transfer-Encoding:");
    slist=curl_slist_append(slist, "Expect:");

    SETOPT(handle, CURLOPT_VERBOSE, 1);
    SETOPT(handle, CURLOPT_URL, env_get_filename(env));
    SETOPT(handle, CURLOPT_READFUNCTION, __readfunc);
    SETOPT(handle, CURLOPT_READDATA, &buf);
    SETOPT(handle, CURLOPT_UPLOAD, 1);
    SETOPT(handle, CURLOPT_PUT, 1);
    SETOPT(handle, CURLOPT_WRITEFUNCTION, __writefunc);
    SETOPT(handle, CURLOPT_WRITEDATA, &buf);
    SETOPT(handle, CURLOPT_HTTPHEADER, slist);

    cc=curl_easy_perform(handle);

    allocator_free(env_get_allocator(env), buf.packed_data);
    curl_slist_free_all(slist);

    if (cc) {
        ham_trace(("network transmission failed: %s", curl_easy_strerror(cc)));
        return (HAM_NETWORK_ERROR);
    }

    cc=curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response);
    if (cc) {
        ham_trace(("network transmission failed: %s", curl_easy_strerror(cc)));
        return (HAM_NETWORK_ERROR);
    }

    if (response!=200) {
        ham_trace(("server returned error %u", response));
        return (HAM_NETWORK_ERROR);
    }

    *reply=buf.wrapper;

    return (0);
}

static ham_status_t 
_remote_fun_create(ham_env_t *env, const char *filename,
            ham_u32_t flags, ham_u32_t mode, const ham_parameter_t *param)
{
    ham_status_t st;
    Ham__ConnectRequest msg;
    Ham__Wrapper wrapper, *reply;
    CURL *handle=curl_easy_init();

    ham__wrapper__init(&wrapper);
    ham__connect_request__init(&msg);
    msg.path=(char *)filename;
    wrapper.type=HAM__WRAPPER__TYPE__CONNECT_REQUEST;
    wrapper.connect_request=&msg;

    st=_perform_request(env, handle, &wrapper, &reply);
    if (st) {
        if (reply)
            ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    ham_assert(reply!=0, (""));
    ham_assert(reply->connect_reply!=0, (""));
    st=reply->connect_reply->status;
    ham__wrapper__free_unpacked(reply, 0);

    if (st==0)
        env_set_curl(env, handle);

    return (st);
}

static ham_status_t 
_remote_fun_open(ham_env_t *env, const char *filename, ham_u32_t flags, 
        const ham_parameter_t *param)
{
    ham_status_t st;
    Ham__ConnectRequest msg;
    Ham__Wrapper wrapper, *reply;
    CURL *handle=curl_easy_init();

    ham__wrapper__init(&wrapper);
    ham__connect_request__init(&msg);
    msg.path=(char *)filename;
    wrapper.type=HAM__WRAPPER__TYPE__CONNECT_REQUEST;
    wrapper.connect_request=&msg;

    st=_perform_request(env, handle, &wrapper, &reply);
    if (st) {
        if (reply)
            ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    ham_assert(reply!=0, (""));
    ham_assert(reply->connect_reply!=0, (""));
    st=reply->connect_reply->status;
    ham__wrapper__free_unpacked(reply, 0);

    if (st==0)
        env_set_curl(env, handle);

    return (st);
}

static ham_status_t
_remote_fun_rename_db(ham_env_t *env, ham_u16_t oldname, 
                ham_u16_t newname, ham_u32_t flags)
{
    ham_status_t st;
    Ham__EnvRenameRequest msg;
    Ham__Wrapper wrapper, *reply;
    
    ham__wrapper__init(&wrapper);
    ham__env_rename_request__init(&msg);
    msg.oldname=oldname;
    msg.newname=newname;
    msg.flags=flags;
    wrapper.type=HAM__WRAPPER__TYPE__ENV_RENAME_REQUEST;
    wrapper.env_rename_request=&msg;

    st=_perform_request(env, env_get_curl(env), &wrapper, &reply);
    if (st) {
        if (reply)
            ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    ham_assert(reply!=0, (""));
    ham_assert(reply->env_rename_reply!=0, (""));
    st=reply->env_rename_reply->status;

    ham__wrapper__free_unpacked(reply, 0);

    return (st);
}

static ham_status_t
_remote_fun_erase_db(ham_env_t *env, ham_u16_t name, ham_u32_t flags)
{
    return (HAM_NOT_IMPLEMENTED);
}

static ham_status_t
_remote_fun_get_database_names(ham_env_t *env, ham_u16_t *names, 
            ham_size_t *count)
{
    ham_status_t st;
    ham_size_t i;
    Ham__EnvGetDatabaseNamesRequest msg;
    Ham__Wrapper wrapper, *reply;
    
    ham__wrapper__init(&wrapper);
    ham__env_get_database_names_request__init(&msg);
    wrapper.type=HAM__WRAPPER__TYPE__ENV_GET_DATABASE_NAMES_REQUEST;
    wrapper.env_get_database_names_request=&msg;

    st=_perform_request(env, env_get_curl(env), &wrapper, &reply);
    if (st) {
        if (reply)
            ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    ham_assert(reply!=0, (""));
    ham_assert(reply->env_get_database_names_reply!=0, (""));
    st=reply->env_get_database_names_reply->status;
    if (st) {
        ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    /* copy the retrieved names */
    for (i=0; i<reply->env_get_database_names_reply->n_names && i<*count; i++) {
        names[i]=reply->env_get_database_names_reply->names[i];
    }

    *count=i;

    ham__wrapper__free_unpacked(reply, 0);

    return (0);
}

static ham_status_t 
_remote_fun_create_db(ham_env_t *env, ham_db_t *db, 
        ham_u16_t dbname, ham_u32_t flags, const ham_parameter_t *param)
{
    ham_status_t st;
    Ham__EnvCreateDbRequest msg;
    Ham__Wrapper wrapper, *reply;
    ham_size_t i=0, num_params=0;
    ham_u32_t *names;
    ham_u64_t *values;
    const ham_parameter_t *p;
    
    /* count number of parameters */
    p=param;
    if (p) {
        for (; p->name; p++) {
            num_params++;
        }
    }

    /* allocate a memory and copy the parameter names */
    names=(ham_u32_t *)allocator_alloc(env_get_allocator(env), 
            num_params*sizeof(ham_u32_t));
    values=(ham_u64_t *)allocator_alloc(env_get_allocator(env), 
            num_params*sizeof(ham_u64_t));
    if (!names || !values)
        return (HAM_OUT_OF_MEMORY);
    p=param;
    if (p) {
        for (; p->name; p++) {
            names[i]=p->name;
            values[i]=p->value;
            i++;
        }
    }

    ham__wrapper__init(&wrapper);
    ham__env_create_db_request__init(&msg);
    msg.dbname=dbname;
    msg.flags=flags;
    msg.param_names=(unsigned *)&names[0];
    msg.n_param_names=num_params;
    msg.param_values=(unsigned long long*)&values[0];
    msg.n_param_values=num_params;
    wrapper.type=HAM__WRAPPER__TYPE__ENV_CREATE_DB_REQUEST;
    wrapper.env_create_db_request=&msg;

    st=_perform_request(env, env_get_curl(env), &wrapper, &reply);
    if (st) {
        if (reply)
            ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    ham_assert(reply!=0, (""));
    ham_assert(reply->env_create_db_reply!=0, (""));
    st=reply->env_create_db_reply->status;
    if (st) {
        ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    db_set_remote_handle(db, reply->env_create_db_reply->db_handle);

    ham__wrapper__free_unpacked(reply, 0);

    return (0);
}

static ham_status_t 
_remote_fun_open_db(ham_env_t *env, ham_db_t *db, 
        ham_u16_t dbname, ham_u32_t flags, const ham_parameter_t *param)
{
    ham_status_t st;
    Ham__EnvOpenDbRequest msg;
    Ham__Wrapper wrapper, *reply;
    ham_size_t i=0, num_params=0;
    ham_u32_t *names;
    ham_u64_t *values;
    const ham_parameter_t *p;
    
    /* count number of parameters */
    p=param;
    if (p) {
        for (; p->name; p++) {
            num_params++;
        }
    }

    /* allocate a memory and copy the parameter names */
    names=(ham_u32_t *)allocator_alloc(env_get_allocator(env), 
            num_params*sizeof(ham_u32_t));
    values=(ham_u64_t *)allocator_alloc(env_get_allocator(env), 
            num_params*sizeof(ham_u64_t));
    if (!names || !values)
        return (HAM_OUT_OF_MEMORY);
    p=param;
    if (p) {
        for (; p->name; p++) {
            names[i]=p->name;
            values[i]=p->value;
            i++;
        }
    }

    ham__wrapper__init(&wrapper);
    ham__env_open_db_request__init(&msg);
    msg.dbname=dbname;
    msg.flags=flags;
    msg.param_names=(unsigned *)&names[0];
    msg.n_param_names=num_params;
    msg.param_values=(unsigned long long*)&values[0];
    msg.n_param_values=num_params;
    wrapper.type=HAM__WRAPPER__TYPE__ENV_OPEN_DB_REQUEST;
    wrapper.env_open_db_request=&msg;

    st=_perform_request(env, env_get_curl(env), &wrapper, &reply);
    if (st) {
        if (reply)
            ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    ham_assert(reply!=0, (""));
    ham_assert(reply->env_open_db_reply!=0, (""));
    st=reply->env_open_db_reply->status;
    if (st) {
        ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    db_set_remote_handle(db, reply->env_open_db_reply->db_handle);

    ham__wrapper__free_unpacked(reply, 0);

    return (0);
}

static ham_status_t
_remote_fun_close(ham_env_t *env, ham_u32_t flags)
{
    (void)flags;

    if (env_get_curl(env)) {
        curl_easy_cleanup(env_get_curl(env));
        env_set_curl(env, 0);
    }

    return (0);
}

static ham_status_t 
_remote_fun_get_parameters(ham_env_t *env, ham_parameter_t *param)
{
    static char filename[1024];
    ham_status_t st;
    Ham__EnvGetParametersRequest msg;
    Ham__Wrapper wrapper, *reply;
    ham_size_t i=0, num_names=0;
    ham_u32_t *names;
    ham_parameter_t *p;
    
    /* count number of parameters */
    p=param;
    if (p) {
        for (; p->name; p++) {
            num_names++;
        }
    }

    /* allocate a memory and copy the parameter names */
    names=(ham_u32_t *)allocator_alloc(env_get_allocator(env), 
            num_names*sizeof(ham_u32_t));
    if (!names)
        return (HAM_OUT_OF_MEMORY);
    p=param;
    if (p) {
        for (; p->name; p++) {
            names[i]=p->name;
            i++;
        }
    }

    ham__wrapper__init(&wrapper);
    ham__env_get_parameters_request__init(&msg);
    msg.names=(unsigned *)&names[0];
    msg.n_names=num_names;
    wrapper.type=HAM__WRAPPER__TYPE__ENV_GET_PARAMETERS_REQUEST;
    wrapper.env_get_parameters_request=&msg;

    st=_perform_request(env, env_get_curl(env), &wrapper, &reply);
    if (st) {
        if (reply)
            ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    ham_assert(reply!=0, (""));
    ham_assert(reply->env_get_parameters_reply!=0, (""));
    st=reply->env_get_parameters_reply->status;
    if (st) {
        ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    p=param;
    while (p && p->name) {
        switch (p->name) {
        case HAM_PARAM_CACHESIZE:
            ham_assert(reply->env_get_parameters_reply->has_cachesize, (""));
            p->value=reply->env_get_parameters_reply->cachesize;
            break;
        case HAM_PARAM_PAGESIZE:
            ham_assert(reply->env_get_parameters_reply->has_pagesize, (""));
            p->value=reply->env_get_parameters_reply->pagesize;
            break;
        case HAM_PARAM_MAX_ENV_DATABASES:
            ham_assert(reply->env_get_parameters_reply->has_max_env_databases, 
                        (""));
            p->value=reply->env_get_parameters_reply->max_env_databases;
            break;
        case HAM_PARAM_GET_FLAGS:
            ham_assert(reply->env_get_parameters_reply->has_flags, (""));
            p->value=reply->env_get_parameters_reply->flags;
            break;
        case HAM_PARAM_GET_FILEMODE:
            ham_assert(reply->env_get_parameters_reply->has_filemode, (""));
            p->value=reply->env_get_parameters_reply->filemode;
            break;
        case HAM_PARAM_GET_FILENAME:
            ham_assert(reply->env_get_parameters_reply->filename, (""));
            strncpy(filename, reply->env_get_parameters_reply->filename, 
                        sizeof(filename));
            p->value=PTR_TO_U64(&filename[0]);
            break;
        default:
            ham_trace(("unknown parameter %d", (int)p->name));
            break;
        }
        p++;
    }

    ham__wrapper__free_unpacked(reply, 0);

    return (0);
}

static ham_status_t
_remote_fun_flush(ham_env_t *env, ham_u32_t flags)
{
    ham_status_t st;
    Ham__EnvFlushRequest msg;
    Ham__Wrapper wrapper, *reply;
    
    ham__wrapper__init(&wrapper);
    ham__env_flush_request__init(&msg);
    msg.flags=flags;
    wrapper.type=HAM__WRAPPER__TYPE__ENV_FLUSH_REQUEST;
    wrapper.env_flush_request=&msg;

    st=_perform_request(env, env_get_curl(env), &wrapper, &reply);
    if (st) {
        if (reply)
            ham__wrapper__free_unpacked(reply, 0);
        return (st);
    }

    ham_assert(reply!=0, (""));
    ham_assert(reply->env_flush_reply!=0, (""));
    st=reply->env_flush_reply->status;

    ham__wrapper__free_unpacked(reply, 0);

    return (st);
}

#endif /* HAM_ENABLE_REMOTE */

ham_status_t
env_initialize_remote(ham_env_t *env)
{
#if HAM_ENABLE_REMOTE
    env->_fun_create             =_remote_fun_create;
    env->_fun_open               =_remote_fun_open;
    env->_fun_rename_db          =_remote_fun_rename_db;
    env->_fun_erase_db           =_remote_fun_erase_db;
    env->_fun_get_database_names =_remote_fun_get_database_names;
    env->_fun_get_parameters     =_remote_fun_get_parameters;
    env->_fun_flush              =_remote_fun_flush;
    env->_fun_create_db          =_remote_fun_create_db;
    env->_fun_open_db            =_remote_fun_open_db;
    env->_fun_close              =_remote_fun_close;

    env_set_rt_flags(env, env_get_rt_flags(env)|DB_IS_REMOTE);
#else
    return (HAM_NOT_IMPLEMENTED);
#endif

    return (0);
}
