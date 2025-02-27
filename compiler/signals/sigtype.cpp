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

#include <climits>
#include <iostream>
#include <sstream>

#include "exception.hh"
#include "global.hh"
#include "property.hh"
#include "sigtype.hh"
#include "tree.hh"

using namespace std;

// Uncomment to activate type inferrence tracing
//#define TRACE(x) x

#define TRACE(x) \
    {            \
        ;        \
    }

AudioType::AudioType(int n, int v, int c, int vec, int b, interval i, res r)
    : fNature(n), fVariability(v), fComputability(c), fVectorability(vec), fBoolean(b), fInterval(i), fRes(r), fCode(0)
{
    TRACE(cerr << gGlobal->TABBER << "Building audioType : n="
               << "NR"[n] << ", v="
               << "KB?S"[v] << ", c="
               << "CI?E"[c] << ", vec="
               << "VS?TS"[vec] << ", b="
               << "N?B"[b] << ", i=" << i << endl);
}  ///< constructs an abstract audio type

bool SimpleType::isMaximal() const  ///< true when type is maximal (and therefore can't change depending of hypothesis)
{
    return (fNature == kReal) && (fVariability == kSamp) && (fComputability == kExec);
}

//------------------------------------------------------------------------------------
//
//		Overloading << printing operator
//
//------------------------------------------------------------------------------------

ostream& operator<<(ostream& dst, const Type& t)
{
    return t->print(dst);
}

ostream& operator<<(ostream& dst, const SimpleType& t)
{
    return t.print(dst);
}

ostream& operator<<(ostream& dst, const TableType& t)
{
    return t.print(dst);
}

ostream& operator<<(ostream& dst, const TupletType& t)
{
    return t.print(dst);
}

//------------------------------------------------------------------------------------
//
//		Print method definition
//
//------------------------------------------------------------------------------------

/**
 * Print the content of a simple type on a stream
 */
ostream& SimpleType::print(ostream& dst) const
{
    return dst << "NR"[nature()] << "KB?S"[variability()] << "CI?E"[computability()] << "VS?TS"[vectorability()]
               << "N?B"[boolean()] << " " << fInterval;
}

/**
 * Print the content of a table type on a stream
 */
ostream& TableType::print(ostream& dst) const
{
    dst << "NR"[nature()] << "KB?S"[variability()] << "CI?E"[computability()] << "VS?TS"[vectorability()]
        << "N?B"[boolean()] << " " << fInterval << ":Table(";
    fContent->print(dst);
    return dst << ')';
}

/**
 *  True when type is maximal (and therefore can't change depending of hypothesis)
 */
bool TableType::isMaximal() const
{
    return (fNature == kReal) && (fVariability == kSamp) && (fComputability == kExec);
}

/**
 * Print the content of a tuplet of types on a stream
 */
ostream& TupletType::print(ostream& dst) const
{
    dst << "KB?S"[variability()] << "CI?E"[computability()] << " " << fInterval << " : {";
    string sep = "";
    for (unsigned int i = 0; i < fComponents.size(); i++, sep = "*") {
        dst << sep;
        fComponents[i]->print(dst);
    }
    dst << '}';
    return dst;
}

/**
 *  True when type is maximal (and therefore can't change depending of hypothesis)
 */
bool TupletType::isMaximal() const
{
    for (unsigned int i = 0; i < fComponents.size(); i++) {
        if (!fComponents[i]->isMaximal()) return false;
    }
    return true;
}

//------------------------------------------------------------------------------------
//
//		Types constructions
// 		t := p, table(t), t|t, t*t
//
//------------------------------------------------------------------------------------

Type operator|(const Type& t1, const Type& t2)
{
    SimpleType *st1, *st2;
    TableType * tt1, *tt2;
    TupletType *nt1, *nt2;

    if ((st1 = isSimpleType(t1)) && (st2 = isSimpleType(t2))) {
        return makeSimpleType(st1->nature() | st2->nature(), st1->variability() | st2->variability(),
                              st1->computability() | st2->computability(), st1->vectorability() | st2->vectorability(),
                              st1->boolean() | st2->boolean(), reunion(st1->getInterval(), st2->getInterval()));

    } else if ((tt1 = isTableType(t1)) && (tt2 = isTableType(t2))) {
        return makeTableType(tt1->content() | tt2->content());

    } else if ((nt1 = isTupletType(t1)) && (nt2 = isTupletType(t2))) {
        vector<Type> v;
        int          n = (int)min(nt1->arity(), nt2->arity());
        for (int i = 0; i < n; i++) {
            v.push_back((*nt1)[i] | (*nt2)[i]);
        }
        return new TupletType(v);

    } else {
        stringstream error;
        error << "ERROR : trying to combine incompatible types, " << t1 << " and " << t2 << endl;
        throw faustexception(error.str());
    }
}

bool operator==(const Type& t1, const Type& t2)
{
    SimpleType *st1, *st2;
    TableType * tt1, *tt2;
    TupletType *nt1, *nt2;

    if (t1->variability() != t2->variability()) return false;
    if (t1->computability() != t2->computability()) return false;

    if ((st1 = isSimpleType(t1)) && (st2 = isSimpleType(t2)))
        return (st1->nature() == st2->nature()) && (st1->variability() == st2->variability()) &&
               (st1->computability() == st2->computability()) && (st1->vectorability() == st2->vectorability()) &&
               (st1->boolean() == st2->boolean()) && (st1->getInterval().lo == st2->getInterval().lo) &&
               (st1->getInterval().hi == st2->getInterval().hi) &&
               (st1->getInterval().valid == st2->getInterval().valid) && st1->getRes().valid == st2->getRes().valid &&
               st1->getRes().index == st2->getRes().index;
    if ((tt1 = isTableType(t1)) && (tt2 = isTableType(t2))) return tt1->content() == tt2->content();
    if ((nt1 = isTupletType(t1)) && (nt2 = isTupletType(t2))) {
        int a1 = nt1->arity();
        int a2 = nt2->arity();
        if (a1 == a2) {
            for (int i = 0; i < a1; i++) {
                if ((*nt1)[i] != (*nt2)[i]) return false;
            }
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool operator<=(const Type& t1, const Type& t2)
{
    return (t1 | t2) == t2;
}

Type operator*(const Type& t1, const Type& t2)
{
    vector<Type> v;

    TupletType* nt1 = dynamic_cast<TupletType*>((AudioType*)t1);
    TupletType* nt2 = dynamic_cast<TupletType*>((AudioType*)t2);

    if (nt1) {
        for (int i = 0; i < nt1->arity(); i++) {
            v.push_back((*nt1)[i]);
        }
    } else {
        v.push_back(t1);
    }

    if (nt2) {
        for (int i = 0; i < nt2->arity(); i++) {
            v.push_back((*nt2)[i]);
        }
    } else {
        v.push_back(t2);
    }
    return new TupletType(v);
}

SimpleType* isSimpleType(AudioType* t)
{
    return dynamic_cast<SimpleType*>(t);
}
TableType* isTableType(AudioType* t)
{
    return dynamic_cast<TableType*>(t);
}
TupletType* isTupletType(AudioType* t)
{
    return dynamic_cast<TupletType*>(t);
}

//--------------------------------------------------
// Type checking

Type checkInt(Type t)
{
    // check that t is an integer
    SimpleType* st = isSimpleType(t);
    if (st == 0 || st->nature() > kInt) {
        stringstream error;
        error << "ERROR : checkInt failed for type " << t << endl;
        throw faustexception(error.str());
    }
    return t;
}

Type checkKonst(Type t)
{
    // check that t is a constant
    if (t->variability() > kKonst) {
        stringstream error;
        error << "ERROR : checkKonst failed for type " << t << endl;
        throw faustexception(error.str());
    }
    return t;
}

Type checkInit(Type t)
{
    // check that t is a known at init time
    if (t->computability() > kInit) {
        stringstream error;
        error << "ERROR : checkInit failed for type " << t << endl;
        throw faustexception(error.str());
    }
    return t;
}

Type checkIntParam(Type t)
{
    return checkInit(checkKonst(checkInt(t)));
}

Type checkWRTbl(Type tbl, Type wr)
{
    // check that wr is compatible with tbl content
    if (wr->nature() > tbl->nature()) {
        stringstream error;
        error << "ERROR : checkWRTbl failed, the content of " << tbl << " is incompatible with " << wr << endl;
        throw faustexception(error.str());
    }
    return tbl;
}

/**
    \brief Check is a type is appropriate for a delay.
    @return an exception if not appropriate, mxd (max delay) if appropriate

 */
int checkDelayInterval(Type t)
{
    interval i = t->getInterval();
    if (i.valid && i.lo >= 0 && i.hi < INT_MAX) {
        return int(i.hi + 0.5);
    } else {
        stringstream error;
        error << "ERROR : invalid delay parameter range: " << i << ". The range must be between 0 and INT_MAX" << endl;
        throw faustexception(error.str());
    }
}

/*****************************************************************************
 *
 *      codeAudioType(Type) -> Tree
 *      Code an audio type as a tree in order to benefit of memoization
 *
 *****************************************************************************/

static Tree codeSimpleType(SimpleType* st);
static Tree codeTableType(TableType* st);
static Tree codeTupletType(TupletType* st);

/**
 * codeAudioType(Type) -> Tree
 * Code an audio type as a tree in order to benefit of memoization
 * The type field (of the coded type) is used to store the audio
 * type
 */
Tree codeAudioType(AudioType* t)
{
    SimpleType* st;
    TableType*  tt;
    TupletType* nt;

    Tree r;

    if ((r = t->getCode())) return r;

    if ((st = isSimpleType(t))) {
        r = codeSimpleType(st);
    } else if ((tt = isTableType(t))) {
        r = codeTableType(tt);
    } else if ((nt = isTupletType(t))) {
        r = codeTupletType(nt);
    } else {
        stringstream error;
        error << "ERROR in codeAudioType() : invalid pointer " << t << endl;
        throw faustexception(error.str());
    }

    r->setType(t);
    return r;
}

/**
 * Code a simple audio type as a tree in order to benefit of memoization
 */
static Tree codeSimpleType(SimpleType* st)
{
    vector<Tree> elems;
    elems.push_back(tree(st->nature()));
    elems.push_back(tree(st->variability()));
    elems.push_back(tree(st->computability()));
    elems.push_back(tree(st->vectorability()));
    elems.push_back(tree(st->boolean()));

    elems.push_back(tree(st->getInterval().valid));
    elems.push_back(tree(st->getInterval().lo));
    elems.push_back(tree(st->getInterval().hi));

    elems.push_back(tree(st->getRes().valid));
    elems.push_back(tree(st->getRes().index));
    return CTree::make(gGlobal->SIMPLETYPE, elems);
}

AudioType* makeSimpleType(int n, int v, int c, int vec, int b, const interval& i)
{
    return makeSimpleType(n, v, c, vec, b, i, gGlobal->RES);
}

AudioType* makeSimpleType(int n, int v, int c, int vec, int b, const interval& i, const res& lsb)
{
    SimpleType prototype(n, v, c, vec, b, i, lsb);
    Tree       code = codeAudioType(&prototype);

    AudioType* t;
    if (gGlobal->gMemoizedTypes->get(code, t)) {
        return t;
    } else {
        gGlobal->gAllocationCount++;
        t = new SimpleType(n, v, c, vec, b, i, lsb);
        gGlobal->gMemoizedTypes->set(code, t);
        t->setCode(code);
        return t;
    }
}

/**
 * Code a table type as a tree in order to benefit of memoization
 */
static Tree codeTableType(TableType* tt)
{
    vector<Tree> elems;
    elems.push_back(tree(tt->nature()));
    elems.push_back(tree(tt->variability()));
    elems.push_back(tree(tt->computability()));
    elems.push_back(tree(tt->vectorability()));
    elems.push_back(tree(tt->boolean()));

    elems.push_back(tree(tt->getInterval().valid));
    elems.push_back(tree(tt->getInterval().lo));
    elems.push_back(tree(tt->getInterval().hi));

    elems.push_back(tree(tt->getRes().valid));
    elems.push_back(tree(tt->getRes().index));

    return CTree::make(gGlobal->TABLETYPE, elems);
}

AudioType* makeTableType(const Type& ct)
{
    TableType prototype(ct);
    Tree      code = codeAudioType(&prototype);

    AudioType* tt;
    if (gGlobal->gMemoizedTypes->get(code, tt)) {
        return tt;
    } else {
        gGlobal->gAllocationCount++;
        tt = new TableType(prototype);
        gGlobal->gMemoizedTypes->set(code, tt);
        tt->setCode(code);
        return tt;
    }
}

AudioType* makeTableType(const Type& ct, int n, int v, int c, int vec, int b, const interval& i)
{
    TableType prototype(ct, n, v, c, vec, b, i);
    Tree      code = codeAudioType(&prototype);

    AudioType* tt;
    if (gGlobal->gMemoizedTypes->get(code, tt)) {
        return tt;
    } else {
        gGlobal->gAllocationCount++;
        tt = new TableType(ct, n, v, c, vec, b, i);
        gGlobal->gMemoizedTypes->set(code, tt);
        tt->setCode(code);
        return tt;
    }
}

/**
 * Code a tuplet type as a tree in order to benefit of memoization
 */
static Tree codeTupletType(TupletType* nt)
{
    vector<Tree> elems;
    for (int i = 0; i < nt->arity(); i++) {
        elems.push_back(codeAudioType((*nt)[i]));
    }
    return CTree::make(gGlobal->TUPLETTYPE, elems);
}

AudioType* makeTupletType(const vector<Type>& vt)
{
    TupletType prototype(vt);
    Tree       code = codeAudioType(&prototype);

    AudioType* t;
    if (gGlobal->gMemoizedTypes->get(code, t)) {
        return t;
    } else {
        gGlobal->gAllocationCount++;
        t = new TupletType(vt);
        gGlobal->gMemoizedTypes->set(code, t);
        t->setCode(code);
        return t;
    }
}

AudioType* makeTupletType(const vector<Type>& vt, int n, int v, int c, int vec, int b, const interval& i)
{
    TupletType prototype(vt, n, v, c, vec, b, i);
    Tree       code = codeAudioType(&prototype);

    AudioType* t;
    if (gGlobal->gMemoizedTypes->get(code, t)) {
        return t;
    } else {
        gGlobal->gAllocationCount++;
        t = new TupletType(vt, n, v, c, vec, b, i);
        gGlobal->gMemoizedTypes->set(code, t);
        t->setCode(code);
        return t;
    }
}
