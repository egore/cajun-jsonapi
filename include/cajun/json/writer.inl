/******************************************************************************

Copyright (c) 2009-2010, Terry Caton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the projecct nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#include "writer.h"
#include <iostream>
#include <iomanip>

/*

TODO:
* better documentation

*/

namespace json
{


inline void Writer::Write(const UnknownElement& elementRoot, std::ostream& ostr) { Write_i(elementRoot, ostr); }
inline void Writer::Write(const Object& object, std::ostream& ostr, bool pretty) { Write_i(object, ostr, pretty); }
inline void Writer::Write(const Array& array, std::ostream& ostr, bool pretty)   { Write_i(array, ostr, pretty); }
inline void Writer::Write(const Number& number, std::ostream& ostr)              { Write_i(number, ostr); }
inline void Writer::Write(const String& string, std::ostream& ostr)              { Write_i(string, ostr); }
inline void Writer::Write(const Boolean& boolean, std::ostream& ostr)            { Write_i(boolean, ostr); }
inline void Writer::Write(const Null& null, std::ostream& ostr)                  { Write_i(null, ostr); }


inline Writer::Writer(std::ostream& ostr, bool pretty) :
   m_ostr(ostr),
   m_pretty(pretty),
   m_nTabDepth(0)
{}

template <typename ElementTypeT>
void Writer::Write_i(const ElementTypeT& element, std::ostream& ostr, bool pretty)
{
   Writer writer(ostr, pretty);
   writer.Write_i(element);
   ostr.flush(); // all done
}

inline void Writer::Write_i(const Array& array)
{
   if (array.Empty())
      m_ostr << "[]";
   else
   {
      m_ostr << '[';
      if(m_pretty) m_ostr << std::endl;
      ++m_nTabDepth;

      Array::const_iterator it(array.Begin()),
                            itEnd(array.End());
      while (it != itEnd) {
         if(m_pretty) m_ostr << std::string(m_nTabDepth, '\t');

         Write_i(*it);

         if (++it != itEnd)
            m_ostr << ',';
         if(m_pretty) m_ostr << std::endl;
      }

      --m_nTabDepth;
      if(m_pretty) m_ostr << std::string(m_nTabDepth, '\t');
      m_ostr << ']';
   }
}

inline void Writer::Write_i(const Object& object)
{
   if (object.Empty())
      m_ostr << "{}";
   else
   {
      m_ostr << '{';
      if(m_pretty) m_ostr << std::endl;
      ++m_nTabDepth;

      Object::const_iterator it(object.Begin()),
                             itEnd(object.End());
      while (it != itEnd) {
         if(m_pretty) m_ostr << std::string(m_nTabDepth, '\t');

         Write_i(it->name);

         m_ostr << (m_pretty ? " : " : ":");
         Write_i(it->element);

         if (++it != itEnd)
            m_ostr << ',';
         if(m_pretty) m_ostr << std::endl;
      }

      --m_nTabDepth;
      if(m_pretty) m_ostr << std::string(m_nTabDepth, '\t');
      m_ostr << '}';
   }
}

inline void Writer::Write_i(const Number& numberElement)
{
   m_ostr << std::dec << std::setprecision(20) << numberElement.Value();
}

inline void Writer::Write_i(const Boolean& booleanElement)
{
   m_ostr << (booleanElement.Value() ? "true" : "false");
}

inline void Writer::Write_i(const String& stringElement)
{
   m_ostr << '"';

   const std::string& s = stringElement.Value();
   std::string::const_iterator it(s.begin()),
                               itEnd(s.end());
   for (; it != itEnd; ++it)
   {
      // check for UTF-8 unicode encoding
      unsigned char u = static_cast<unsigned char>(*it);
      if (u & 0xc0) {
         if ((u & 0xe0) == 0xc0) {
            // two-character sequence
            int x = (*it & 0x1f) << 6;
            if ((it + 1) == itEnd) {
               m_ostr << *it; continue;
            }
            u = static_cast<unsigned char>(*(it + 1));
            if ((u & 0xc0) == 0x80) {
               x |= u & 0x3f;
               m_ostr << "\\u" << std::hex << std::setfill('0')
                  << std::setw(4) << x;
               ++it;
               continue;
            }

         } else if ((u & 0xf0) == 0xe0) {
            // three-character sequence
            int x = (u & 0x0f) << 12;
            if ((it + 1) == itEnd) {
               m_ostr << *it; continue;
            }
            u = static_cast<unsigned char>(*(it + 1));
            if ((u & 0xc0) == 0x80) {
               x |= (u & 0x3f) << 6;
               if ((it + 2) == itEnd) {
                  m_ostr << *it; continue;
               }
               u = static_cast<unsigned char>(*(it + 2));
               if ((u & 0xc0) == 0x80) {
                  x |= u & 0x3f;
                  m_ostr << "\\u" << std::hex << std::setfill('0')
                     << std::setw(4) << x;
                  it = it + 2;
                  continue;
               }
            }
         }
      }

      switch (*it)
      {
         case '"':         m_ostr << "\\\"";   break;
         case '\\':        m_ostr << "\\\\";   break;
         case '\b':        m_ostr << "\\b";    break;
         case '\f':        m_ostr << "\\f";    break;
         case '\n':        m_ostr << "\\n";    break;
         case '\r':        m_ostr << "\\r";    break;
         case '\t':        m_ostr << "\\t";    break;
         default:          m_ostr << *it;      break;
      }
   }

   m_ostr << '"';
}

inline void Writer::Write_i(const Null& )
{
   m_ostr << "null";
}

inline void Writer::Write_i(const UnknownElement& unknown)
{
   unknown.Accept(*this);
}

inline void Writer::Visit(const Array& array)       { Write_i(array); }
inline void Writer::Visit(const Object& object)     { Write_i(object); }
inline void Writer::Visit(const Number& number)     { Write_i(number); }
inline void Writer::Visit(const String& string)     { Write_i(string); }
inline void Writer::Visit(const Boolean& boolean)   { Write_i(boolean); }
inline void Writer::Visit(const Null& null)         { Write_i(null); }



} // End namespace
