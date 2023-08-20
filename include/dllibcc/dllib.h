/******************************************************************************************
 *   Copyright (C) 1965/1987/2023 by Serge Lussier                                        *
 *   serge.lussier@oldlonecoder.club                                                      *
 *                                                                                        *
 *                                                                                        *
 *   Unless otherwise specified, all code in this project is written                      *
 *   by the author (Serge Lussier)                                                        *
 *   and no one else then not even {copilot, chatgpt, or any other AI calamities}         *
 *   ----------------------------------------------------------------------------------   *
 *   Copyrights from authors other than Serge Lussier also apply here                     *
 *   Open source FREE licences also apply to the code from the same author (Serge Lussier)*
 *   ----------------------------------------------------------------------------------   *
 ******************************************************************************************/


#pragma once


#include <dllibcc/dllib.h>
#include <logbook/rem.h>
#include 


#ifdef _WIN32
#   include <Windows.h>
#else
#   define FARPROC     void*
#   define HINSTANCE   void*

#   include <dlfcn.h>
#endif

#include <map>


#define BEGIN_EXPORT      extern "C"\
{
#define ENDEXPORT \
}

#define CREATE_SYM "_add_symbol"
#define DELETE_SYM "_remove_symbol"
#define EXPORT_SYM "_export"
#define EXPORT   dll::dllib::interface_map export()
#define CREATE_INSTANCE dll::dllib* _create(book::object* parent_obj, const std::string &obj_id)
#define DELETE_INSTANCE void _del(dll::dllib* dllib)


namespace dll
{


class DLLIB_PUBLIC dllib: public book::object
{
    
    friend class dll_file;
public:
    using interface_map = std::map<std::string, FARPROC>;
protected:
    interface_map _interface;
public:
   dllib() = delete;
   dllib(constdllib&) = delete;
   dllib(object* parent_, const std::string& id_);
    ~rtdl() override;
    

    template<typename return_type, typename ...Args> expect<return_type> call(const std::string& symbol_id, Args...args)
    {
        using F = reutrn_type(*)(dllib*, Args...args);
        auto fn_i = _interface.find(symbol_id);
        if (fn_i == _interface.end())
        {
            return book::rem::push_error(HERE) << book::rem::code::notexist << " symbol '" << symbol_id;
            //return return_type();
        }
        F fn = reinterpret_cast<F>(f->second);
        if (fn)
            return fn(this, Args_...);
    }

    void set_interface(const dllib::interface_map& i_) { _interface = i_; }
    virtual rem::code Run() = 0;
};




class DLLIB_PUBLIC dll_file  final : public book::object
{
   HINSTANCE _Handle = nullptr;
   std::string _id;
   static dllib::interface_map _null;
   dllib* _dl = nullptr;
   dllib::interface_map _interface;

public:

    dll_file() {}
    dll_file(book::object* parent_obj, const std::string& dl_id);
    ~dll_file() = default;
    std::string locate();
    rem::code open();
    int close();
    int release();
};


}
