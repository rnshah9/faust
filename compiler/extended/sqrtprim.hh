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
#include "floats.hh"
#include "xtended.hh"

class SqrtPrim : public xtended {
   public:
    SqrtPrim() : xtended("sqrt") {}

    virtual unsigned int arity() { return 1; }

    virtual bool needCache() { return true; }

    virtual ::Type infereSigType(const vector<::Type>& args)
    {
        faustassert(args.size() == 1);
        Type     t = args[0];
        interval i = t->getInterval();
        if (i.valid) {
            if (i.lo >= 0) {
                return castInterval(floatCast(t), interval(sqrt(i.lo), sqrt(i.hi)));
            } else if (gGlobal->gMathExceptions) {
                cerr << "WARNING : potential out of domain in sqrt(" << i << ")" << endl;
            }
        }
        return castInterval(floatCast(t), interval());
    }

    virtual int infereSigOrder(const vector<int>& args) { return args[0]; }

    virtual Tree computeSigOutput(const vector<Tree>& args)
    {
        // check simplifications
        num n;
        if (isNum(args[0], n)) {
            if (double(n) < 0) {
                stringstream error;
                error << "ERROR : out of domain sqrt(" << ppsig(args[0]) << ")" << endl;
                throw faustexception(error.str());
            } else {
                return tree(sqrt(double(n)));
            }
        } else {
            return tree(symbol(), args[0]);
        }
    }

    virtual ValueInst* generateCode(CodeContainer* container, Values& args, ::Type result,
                                    vector< ::Type> const& types)
    {
        faustassert(args.size() == arity());
        faustassert(types.size() == arity());

        Typed::VarType         result_type;
        vector<Typed::VarType> arg_types;
        Values       casted_args;
        prepareTypeArgsResult(result, args, types, result_type, arg_types, casted_args);

        return container->pushFunction(subst("sqrt$0", isuffix()), result_type, arg_types, casted_args);
    }

    virtual string generateCode(Klass* klass, const vector<string>& args, const vector<::Type>& types)
    {
        faustassert(args.size() == arity());
        faustassert(types.size() == arity());

        return subst("sqrt$1($0)", args[0], isuffix());
    }

    virtual string generateLateq(Lateq* lateq, const vector<string>& args, const vector<::Type>& types)
    {
        faustassert(args.size() == arity());
        faustassert(types.size() == arity());

        return subst("\\sqrt{$0}", args[0]);
    }
};
