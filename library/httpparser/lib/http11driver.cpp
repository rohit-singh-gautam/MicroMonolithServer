/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http11driver.h>
#include <sstream>

rohit::http11driver::~http11driver()
{
   delete(scanner);
   scanner = nullptr;
   delete(parser);
   parser = nullptr;
}

rohit::err_t rohit::http11driver::parse(std::string &text) {
    std::stringstream textstream(text);
    return parse_internal(textstream);
}

rohit::err_t rohit::http11driver::parse_internal(std::istream &textstream) {
    delete(scanner);
    try
    {
        scanner = new http11scanner( &textstream );
    }
    catch( std::bad_alloc &exception )
    {
#ifdef DEBUG
        std::cerr << "Failed to allocate scanner: (" <<
                exception.what() << ")\n";
#endif
        throw rohit::exception_t(rohit::err_t::HTTP11_PARSER_MEMORY_FAILURE);
    }

    delete(parser); 
    try
    {
        parser = new rohit::parser(*scanner, *this);
    }
    catch( std::bad_alloc &ba )
    {
        std::cerr << "Failed to allocate parser: (" << 
            ba.what() << "), exiting!!\n";
        exit( EXIT_FAILURE );
    }

    scanner->BEGIN_REQUEST();
    const int accept( 0 );
    if( parser->parse() != accept )
    {
        return rohit::err_t::HTTP11_PARSER_FAILURE;
    }
    return rohit::err_t::SUCCESS;
}

