/* SLiM - Simple Login Manager
   Copyright (C) 2007 Martin Parm

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _PAM_H_
#define _PAM_H_
#include <string>
#include <security/pam_appl.h>

#ifdef __LIBPAM_VERSION
#include <security/pam_misc.h>
#endif

namespace PAM {
    class Exception{
    public:
        int errnum;
        std::string errstr;
        std::string func_name;
        Exception(pam_handle_t* _pam_handle,
                  const std::string& _func_name,
                  int _errnum);
        virtual ~Exception(void);
    };

    class Auth_Exception: public Exception{
    public:
        Auth_Exception(pam_handle_t* _pam_handle,
                       const std::string& _func_name,
                       int _errnum);
    };

    class Cred_Exception: public Exception{
    public:
        Cred_Exception(pam_handle_t* _pam_handle,
                       const std::string& _func_name,
                       int _errnum);
    };


    class Authenticator{
    private:
        struct pam_conv pam_conversation;
        pam_handle_t* pam_handle;
        int last_result;

        int _end(void);
    public:
        typedef int (conversation)(int num_msg,
                           const struct pam_message **msg,
                           struct pam_response **resp,
                           void *appdata_ptr);

        enum ItemType {
            Service     = PAM_SERVICE,
            User        = PAM_USER,
            User_Prompt = PAM_USER_PROMPT,
            TTY         = PAM_TTY,
            Requestor   = PAM_RUSER,
            Host        = PAM_RHOST,
            Conv        = PAM_CONV,
#ifdef __LIBPAM_VERSION
            //Fail_Delay  = PAM_FAIL_DELAY
#endif
        };

    public:
        Authenticator(conversation* conv, void* data=0);
        ~Authenticator(void);
        
        void start(const std::string& service);
        void end(void);
        void set_item(const ItemType item, const void* value);
        const void* get_item(const ItemType item);
#ifdef __LIBPAM_VERSION
        void fail_delay(const unsigned int micro_sec);
#endif
        void authenticate(void);
        void open_session(void);
        void close_session(void);
        void setenv(const std::string& key, const std::string& value);
        void delenv(const std::string& key);
        const char* getenv(const std::string& key);
        char** getenvlist(void);

    private:
        // Explicitly disable copy constructor and copy assignment
        Authenticator(const PAM::Authenticator&);
        Authenticator& operator=(const PAM::Authenticator&);
    };
};

std::ostream& operator<<( std::ostream& os, const PAM::Exception& e);
#endif
