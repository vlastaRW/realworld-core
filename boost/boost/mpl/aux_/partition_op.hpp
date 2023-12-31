
#ifndef BOOST_MPL_AUX_PARTITION_OP_HPP_INCLUDED
#define BOOST_MPL_AUX_PARTITION_OP_HPP_INCLUDED

// Copyright Eric Friedman 2003
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/boost/boost/boost/mpl/aux_/partition_op.hpp,v $
// $Date: 2004/09/02 15:40:44 $
// $Revision: 1.3 $

#include <boost/mpl/apply.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/aux_/lambda_spec.hpp>

namespace boost { namespace mpl { 

namespace aux {

template <typename Predicate>
struct partition_op
{
    template <typename State, typename Iter>
    struct apply
    {
    private:
        typedef typename State::first first_;
        typedef typename State::second second_;
        typedef typename deref<Iter>::type t_;
        typedef typename apply1< Predicate,t_ >::type pred_;

        typedef typename eval_if<
              pred_
            , push_front< first_, t_ >
            , push_front< second_, t_ >
            >::type result_;

    public:
        typedef typename if_<
              pred_
            , pair< result_,second_ >
            , pair< first_,result_ >
            >::type type;
    };
};

} // namespace aux

BOOST_MPL_AUX_PASS_THROUGH_LAMBDA_SPEC(1,aux::partition_op)

}}

#endif // BOOST_MPL_AUX_PARTITION_OP_HPP_INCLUDED
