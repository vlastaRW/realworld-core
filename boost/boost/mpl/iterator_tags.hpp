
#ifndef BOOST_MPL_ITERATOR_TAG_HPP_INCLUDED
#define BOOST_MPL_ITERATOR_TAG_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/boost/boost/boost/mpl/iterator_tags.hpp,v $
// $Date: 2004/09/02 15:40:41 $
// $Revision: 1.2 $

#include <boost/mpl/int.hpp>

namespace boost { namespace mpl {

struct forward_iterator_tag : int_<0> {};
struct bidirectional_iterator_tag : int_<1> {};
struct random_access_iterator_tag : int_<2> {};

}}

#endif // BOOST_MPL_ITERATOR_TAG_HPP_INCLUDED
