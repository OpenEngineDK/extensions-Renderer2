// Functional style box template
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_FUNCTIONAL_BOX_H_
#define _OE_FUNCTIONAL_BOX_H_

namespace OpenEngine {
    namespace Utils {

template <class T>
class Box {
private:
    T content;
public:
    Box() {}
    Box(T content): content(content) {} 
    virtual ~Box() {}
    T Get() { return content; }
    void Set(T content) { this->content = content; }
    
};

} // NS Utils
} // NS OpenEngine

#endif // _OE_FUNCTIONAL_BOX_H_
