
#ifndef BOOST_MPL_AUX_ITERATOR_NAMES_HPP_INCLUDED
#define BOOST_MPL_AUX_ITERATOR_NAMES_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/boost/boost/boost/mpl/aux_/iterator_names.hpp,v $
// $Date: 2004/09/02 15:40:43 $
// $Revision: 1.4 $

#include <boost/mpl/aux_/config/workaround.hpp>
#include <boost/mpl/aux_/config/msvc.hpp>

#if !BOOST_WORKAROUND(BOOST_MSVC, <= 1300)
#   define BOOST_MPL_AUX_ITERATOR_ADVANCE advance
#   define BOOST_MPL_AUX_ITERATOR_DISTANCE distance_to
#else
#   define BOOST_MPL_AUX_ITERATOR_ADVANCE advance_
#   define BOOST_MPL_AUX_ITERATOR_DISTANCE distance_to
#endif

#endif // BOOST_MPL_AUX_ITERATOR_NAMES_HPP_INCLUDED
