/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include <http11parser.h>
#include <location.hh>

namespace rohit {

class http11scanner : public yyFlexLexer{
protected:
   using yyFlexLexer::yy_push_state;
   using yyFlexLexer::yy_pop_state;
   using yyFlexLexer::yy_top_state;

public:
   using yyFlexLexer::yyFlexLexer;
   
   virtual ~http11scanner() {};

   using FlexLexer::yylex;

   int yylex(parser::semantic_type * const lval, parser::location_type *location);

   void BEGIN_REQUEST();

   void END();

private:
   rohit::parser::semantic_type *yylval = nullptr;

};

} // namespace rohit