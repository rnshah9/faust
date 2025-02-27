/************************************************************************
 ************************************************************************
    FAUST compiler
    Copyright (C) 2016 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ************************************************************************
 ************************************************************************/

#ifndef __dsp_factory_base__
#define __dsp_factory_base__

#include <string.h>
#include <ostream>
#include <string>

#include "faust/export.h"
#include "faust/gui/CInterface.h"
#include "faust/gui/meta.h"
#include "faust/dsp/dsp.h"

#include "exception.hh"

#define COMPILATION_OPTIONS_KEY "compile_options"
#define COMPILATION_OPTIONS "declare compile_options "

/*
 In order to better separate compilation and execution for dynamic backends (LLVM, Interpreter, WebAssembly).
 A dsp_factory_base* object will either be generated by the compiler from a dsp,
 or by reading an already compiled dsp (in LLVM IR, Interpreter or WebAssembly bytecode).
 */

struct dsp_factory_base {
 
    virtual ~dsp_factory_base() {}
    
    virtual void write(std::ostream* out, bool binary = false, bool compact = false) = 0;
    
    virtual void writeHelper(std::ostream* out, bool binary = false, bool compact = false) {}  // Helper functions

    virtual std::string getName()                        = 0;
    virtual void        setName(const std::string& name) = 0;

    virtual std::string getSHAKey()                           = 0;
    virtual void        setSHAKey(const std::string& sha_key) = 0;

    virtual std::string getDSPCode()                        = 0;
    virtual void        setDSPCode(const std::string& code) = 0;
    
    virtual std::string getCompileOptions() = 0;

    virtual dsp* createDSPInstance(dsp_factory* factory) = 0;

    virtual void                setMemoryManager(dsp_memory_manager* manager) = 0;
    virtual dsp_memory_manager* getMemoryManager()                            = 0;

    virtual void* allocate(size_t size) = 0;
    virtual void  destroy(void* ptr)    = 0;

    virtual void metadata(Meta* meta) = 0;
   
    virtual std::string getBinaryCode() = 0;

    // Sub-classes will typically implement this method to create a factory from a stream
    static dsp_factory_base* read(std::istream* in) { return nullptr; }
};

class dsp_factory_imp : public dsp_factory_base {
   protected:
    std::string         fName;
    std::string         fSHAKey;
    std::string         fExpandedDSP;
    dsp_memory_manager* fManager;

   public:
    dsp_factory_imp(const std::string& name, const std::string& sha_key, const std::string& dsp)
        : fName(name), fSHAKey(sha_key), fExpandedDSP(dsp), fManager(nullptr)
    {
    }

    virtual ~dsp_factory_imp() {}

    std::string getName()
    {
        struct MyMeta : public Meta {
            std::string  name;
            virtual void declare(const char* key, const char* value)
            {
                if (strcmp(key, "name") == 0) name = value;
            }
        };

        MyMeta meta_data;
        metadata(&meta_data);
        return (meta_data.name != "") ? meta_data.name : fName;
    }

    void setName(const std::string& name) { fName = name; }

    std::string getSHAKey() { return fSHAKey; }
    void        setSHAKey(const std::string& sha_key) { fSHAKey = sha_key; }

    std::string getDSPCode() { return fExpandedDSP; }
    void        setDSPCode(const std::string& code) { fExpandedDSP = code; }
    
    virtual std::string getCompileOptions() { return ""; };

    virtual dsp* createDSPInstance(dsp_factory* factory)
    {
        faustassert(false);
        return nullptr;
    }

    virtual void                setMemoryManager(dsp_memory_manager* manager) { fManager = manager; }
    virtual dsp_memory_manager* getMemoryManager() { return fManager; }

    virtual void* allocate(size_t size)
    {
        if (fManager) {
            return fManager->allocate(size);
        } else {
            faustassert(false);
            return nullptr;
        }
    }

    virtual void destroy(void* ptr)
    {
        if (fManager) {
            fManager->destroy(ptr);
        } else {
            faustassert(false);
        }
    }
  
    virtual void metadata(Meta* meta) { faustassert(false); }

    virtual void write(std::ostream* out, bool binary = false, bool compact = false) {}

    virtual std::string getBinaryCode() { return ""; }
};

/* To be used by textual backends */
class text_dsp_factory_aux : public dsp_factory_imp {
   protected:
    std::string fCode;
    std::string fHelpers;

   public:
    text_dsp_factory_aux(const std::string& name, const std::string& sha_key, const std::string& dsp,
                         const std::string& code, const std::string& helpers)
        : dsp_factory_imp(name, sha_key, dsp), fCode(code), fHelpers(helpers)
    {
    }

    virtual void write(std::ostream* out, bool binary = false, bool compact = false) { *out << fCode; }

    virtual void writeHelper(std::ostream* out, bool binary = false, bool compact = false) { *out << fHelpers; }

    virtual std::string getBinaryCode() { return fCode; }
};

// Backend API implemented in libcode.cpp

class CTree;
typedef CTree* Signal;
typedef std::vector<Signal> tvec;

dsp_factory_base* createFactory(const char* name, const char* input,
                                int argc, const char* argv[],
                                std::string& error_msg, bool generate);

dsp_factory_base* createFactory(const char* name, tvec signals,
                                int argc, const char* argv[],
                                std::string& error_msg);

std::string expandDSP(int argc, const char* argv[], const char* name,
                      const char* input, std::string& sha_key,
                      std::string& error_msg);

#endif
