/************************************************************************
 ************************************************************************
    FAUST compiler
    Copyright (C) 2003-2018 GRAME, Centre National de Creation Musicale
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

#include <math.h>

#include "Text.hh"
#include "code_container.hh"
#include "floats.hh"
#include "sigtyperules.hh"
#include "xtended.hh"

class AbsPrim : public xtended {
   public:
    AbsPrim() : xtended("abs") {}

    virtual unsigned int arity() { return 1; }

    virtual bool needCache() { return true; }

    virtual ::Type infereSigType(const vector<::Type>& types)
    {
        faustassert(types.size() == arity());
        Type t = types[0];
        return castInterval(t, abs(t->getInterval()));
        return t;
    }

    virtual int infereSigOrder(const vector<int>& args)
    {
        faustassert(args.size() == arity());
        return args[0];
    }

    virtual Tree computeSigOutput(const vector<Tree>& args)
    {
        double f;
        int    i;
        faustassert(args.size() == arity());
    
        // abs(abs(sig)) ==> abs(sig)
        xtended* xt = (xtended*)getUserData(args[0]);
        if (xt == gGlobal->gAbsPrim) {
            return args[0];
            
        } else if (isDouble(args[0]->node(), &f)) {
            return tree(fabs(f));

        } else if (isInt(args[0]->node(), &i)) {
            return tree(abs(i));

        } else {
            return tree(symbol(), args[0]);
        }
    }

    virtual ValueInst* generateCode(CodeContainer* container, Values& args, ::Type result,
                                    vector<::Type> const& types)
    {
        faustassert(args.size() == arity());
        faustassert(types.size() == arity());

        Typed::VarType         result_type;
        vector<Typed::VarType> arg_types;

        ::Type t = infereSigType(types);
        interval i = types[0]->getInterval();
     
        /*
         04/25/22 : this optimisation cannot be done because interval computation is buggy: like no.noise interval [O..inf] !
         */
    
        /*
            if (i.valid && i.lo >= 0) {
                return *args.begin();
            } else {
                // Only compute abs when arg is < 0
                if (t->nature() == kReal) {
                    Values casted_args;
                    prepareTypeArgsResult(result, args, types, result_type, arg_types, casted_args);
                    return container->pushFunction(subst("fabs$0", isuffix()), result_type, arg_types, casted_args);
                } else {
                    // "Int" abs
                    result_type = Typed::kInt32;
                    arg_types.push_back(Typed::kInt32);
                    return container->pushFunction("abs", result_type, arg_types, args);
                }
            }
        */
    
        if (t->nature() == kReal) {
            Values casted_args;
            prepareTypeArgsResult(result, args, types, result_type, arg_types, casted_args);
            return container->pushFunction(subst("fabs$0", isuffix()), result_type, arg_types, casted_args);
        } else {
            // "Int" abs
            result_type = Typed::kInt32;
            arg_types.push_back(Typed::kInt32);
            return container->pushFunction("abs", result_type, arg_types, args);
        }
    }

    virtual string generateCode(Klass* klass, const vector<string>& args, const vector<::Type>& types)
    {
        faustassert(args.size() == arity());
        faustassert(types.size() == arity());

        Type t = infereSigType(types);
        if (t->nature() == kReal) {
            return subst("fabs$1($0)", args[0], isuffix());
        } else {
            return subst("abs($0)", args[0]);
        }
    }

    virtual string generateLateq(Lateq* lateq, const vector<string>& args, const vector<::Type>& types)
    {
        faustassert(args.size() == arity());
        faustassert(types.size() == arity());

        ::Type t = infereSigType(types);
        return subst("\\left\\lvert{$0}\\right\\rvert", args[0]);
    }
};
