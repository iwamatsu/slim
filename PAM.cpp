/* SLiM - Simple Login Manager
   Copyright (C) 2007 Martin Parm

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/
#include <string>
#include <iostream>
#include "PAM.h"

namespace PAM {
    Exception::Exception(pam_handle_t* _pam_handle,
               const std::string& _func_name,
               int _errnum):
        errnum(_errnum),
        errstr(pam_strerror(_pam_handle, _errnum)),
        func_name(_func_name)
        {}

    Exception::~Exception(void){}

    Auth_Exception::Auth_Exception(pam_handle_t* _pam_handle,
                                   const std::string& _func_name,
                                   int _errnum):
        Exception(_pam_handle, _func_name, _errnum){}

    Cred_Exception::Cred_Exception(pam_handle_t* _pam_handle,
                                   const std::string& _func_name,
                                   int _errnum):
        Exception(_pam_handle, _func_name, _errnum){}

    int Authenticator::_end(void){
        int result=pam_end(pam_handle, last_result);
        pam_handle=0;
        return result;
    }

    Authenticator::Authenticator(conversation* conv, void* data):
        pam_handle(0),
        last_result(PAM_SUCCESS)
    {
        pam_conversation.conv=conv;
        pam_conversation.appdata_ptr=data;
    }

    Authenticator::~Authenticator(void){
        if (pam_handle) _end();
    }

    void Authenticator::start(const std::string& service){
        switch((last_result=pam_start(service.c_str(), NULL, &pam_conversation, &pam_handle))){
            default:
                throw Exception(pam_handle, "pam_start()", last_result);

            case PAM_SUCCESS:
                break;
        }
        return;
    }

    void Authenticator::end(void){
        switch((last_result=_end())){
            default:
                throw Exception(pam_handle, "pam_end()", last_result);

            case PAM_SUCCESS:
                break;

        }
        return;
    }

    void Authenticator::set_item(const Authenticator::ItemType item, const void* value){
        switch((last_result=pam_set_item(pam_handle, item, value))){
            default:
            _end();
                throw Exception(pam_handle, "pam_set_item()", last_result);

            case PAM_SUCCESS:
                break;
        }
        return;
    }

    const void* Authenticator::get_item(const Authenticator::ItemType item){
        const void* data;
        switch ((last_result=pam_get_item(pam_handle, item, &data))){
            default:
            case PAM_SYSTEM_ERR:
#ifdef __LIBPAM_VERSION
            case PAM_BAD_ITEM:
#endif
                _end();
                throw Exception(pam_handle, "pam_get_item()", last_result);

            case PAM_PERM_DENIED: // The value of item was NULL
            case PAM_SUCCESS:
                break;
        }
        return data;
    }

#ifdef __LIBPAM_VERSION
    void Authenticator::fail_delay(const unsigned int micro_sec){
        switch((last_result=pam_fail_delay(pam_handle, micro_sec))){
            default:
                _end();
                throw Exception(pam_handle, "fail_delay()", last_result);

            case PAM_SUCCESS:
                break;
        }
        return;
    }
#endif

    void Authenticator::authenticate(void){
        switch((last_result=pam_authenticate(pam_handle, 0))){
            default:
            case PAM_ABORT:
            case PAM_AUTHINFO_UNAVAIL:
                _end();
                throw Exception(pam_handle, "pam_authenticate()", last_result);

            case PAM_USER_UNKNOWN:
            case PAM_MAXTRIES:
            case PAM_CRED_INSUFFICIENT:
            case PAM_AUTH_ERR:
                throw Auth_Exception(pam_handle, "pam_authentication()", last_result);

            case PAM_SUCCESS:
                break;
        }

        switch((last_result=pam_acct_mgmt(pam_handle, PAM_SILENT))){
            // The documentation and implementation of Linux PAM differs:
            // PAM_NEW_AUTHTOKEN_REQD is described in the documentation but
            // don't exists in the actual implementation. This issue needs
            // to be fixes at some point.

            default:
            //case PAM_NEW_AUTHTOKEN_REQD:
            case PAM_ACCT_EXPIRED:
            case PAM_USER_UNKNOWN:
                _end();
                throw Exception(pam_handle, "pam_acct_mgmt()", last_result);
                
            case PAM_AUTH_ERR:
            case PAM_PERM_DENIED:
                throw Auth_Exception(pam_handle, "pam_acct_mgmt()", last_result);

            case PAM_SUCCESS:
                break;
        };
        return;
    }

    void Authenticator::open_session(void){
        switch((last_result=pam_setcred(pam_handle, PAM_ESTABLISH_CRED))){
            default:
            case PAM_CRED_ERR:
            case PAM_CRED_UNAVAIL:
                _end();
                throw Exception(pam_handle, "pam_setcred()", last_result);

            case PAM_CRED_EXPIRED:
            case PAM_USER_UNKNOWN:
                throw Cred_Exception(pam_handle, "pam_setcred()", last_result);

            case PAM_SUCCESS:
                break;
        }

        switch((last_result=pam_open_session(pam_handle, 0))){
            // The documentation and implementation of Linux PAM differs:
            // PAM_SESSION_ERROR is described in the documentation but
            // don't exists in the actual implementation. This issue needs
            // to be fixes at some point.

            default:
            //case PAM_SESSION_ERROR:
                pam_setcred(pam_handle, PAM_DELETE_CRED);
                _end();
                throw Exception(pam_handle, "pam_open_session()", last_result);

            case PAM_SUCCESS:
                break;
        };
        return;
    }

    void Authenticator::close_session(void){
        switch((last_result=pam_close_session(pam_handle, 0))){
            // The documentation and implementation of Linux PAM differs:
            // PAM_SESSION_ERROR is described in the documentation but
            // don't exists in the actual implementation. This issue needs
            // to be fixes at some point.

            default:
            //case PAM_SESSION_ERROR:
                pam_setcred(pam_handle, PAM_DELETE_CRED);
                _end();
                throw Exception(pam_handle, "pam_close_session", last_result);

            case PAM_SUCCESS:
                break;
        };
        switch((last_result=pam_setcred(pam_handle, PAM_DELETE_CRED))){
            default:
            case PAM_CRED_ERR:
            case PAM_CRED_UNAVAIL:
            case PAM_CRED_EXPIRED:
            case PAM_USER_UNKNOWN:
                _end();
                throw Exception(pam_handle, "pam_setcred()", last_result);

            case PAM_SUCCESS:
                break;
        }
        return;
    }

    void Authenticator::setenv(const std::string& key, const std::string& value){
        std::string name_value = key+"="+value;
        switch((last_result=pam_putenv(pam_handle, name_value.c_str()))){
            default:
            case PAM_PERM_DENIED:
            case PAM_ABORT:
            case PAM_BUF_ERR:
#ifdef __LIBPAM_VERSION
            case PAM_BAD_ITEM:
#endif
                _end();
                throw Exception(pam_handle, "pam_putenv()", last_result);

            case PAM_SUCCESS:
                break;
        };
        return;
    }

    void Authenticator::delenv(const std::string& key){
        switch((last_result=pam_putenv(pam_handle, key.c_str()))){
            default:
            case PAM_PERM_DENIED:
            case PAM_ABORT:
            case PAM_BUF_ERR:
#ifdef __LIBPAM_VERSION
            case PAM_BAD_ITEM:
#endif
                _end();
                throw Exception(pam_handle, "pam_putenv()", last_result);

            case PAM_SUCCESS:
                break;
        };
        return;
    }

    const char* Authenticator::getenv(const std::string& key){
        return pam_getenv(pam_handle, key.c_str());
    }

    char** Authenticator::getenvlist(void){
        return pam_getenvlist(pam_handle);
    }

};

std::ostream& operator<<( std::ostream& os, const PAM::Exception& e){
    os << e.func_name << ": " << e.errstr;
    return os;
}
