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

#ifndef _FAUSTFLOATS_
#define _FAUSTFLOATS_

#include <iostream>
#include <float.h>

#include "instructions.hh"

#define FLOATMACRO "FAUSTFLOAT"
#define FLOATMACROPTR "FAUSTFLOAT*"
#define FLOATMACROPTRPTR "FAUSTFLOAT**"
#define FLOATCASTER "(" FLOATMACRO ")"

const char* isuffix();
const char* inumix();
double      inummin();

int            ifloatsize();
const char*    ifloat();
const char*    ifloatptr();
const char*    ifloatptrptr();
Typed::VarType itfloat();
Typed::VarType itfloatptr();
Typed::VarType itfloatptrptr();
const char*    icast();

const char* xfloat();
const char* xcast();

void initFaustFloat();

void printfloatdef(std::ostream& fout);

typedef long double quad;

#endif
