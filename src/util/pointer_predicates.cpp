/*******************************************************************\

Module: Various predicates over pointers in programs

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

/// \file
/// Various predicates over pointers in programs

#include "pointer_predicates.h"

#include "arith_tools.h"
#include "c_types.h"
#include "cprover_prefix.h"
#include "namespace.h"
#include "pointer_expr.h"
#include "pointer_offset_size.h"
#include "std_expr.h"
#include "symbol.h"

exprt pointer_object(const exprt &p)
{
  return pointer_object_exprt(p, size_type());
}

exprt same_object(const exprt &p1, const exprt &p2)
{
  return equal_exprt(pointer_object(p1), pointer_object(p2));
}

exprt object_size(const exprt &pointer)
{
  return object_size_exprt(pointer, size_type());
}

exprt pointer_offset(const exprt &pointer)
{
  return pointer_offset_exprt(pointer, signed_size_type());
}

exprt deallocated(const exprt &pointer, const namespacet &ns)
{
  // we check __CPROVER_deallocated!
  const symbolt &deallocated_symbol=ns.lookup(CPROVER_PREFIX "deallocated");

  return same_object(pointer, deallocated_symbol.symbol_expr());
}

exprt dead_object(const exprt &pointer, const namespacet &ns)
{
  // we check __CPROVER_dead_object!
  const symbolt &deallocated_symbol=ns.lookup(CPROVER_PREFIX "dead_object");

  return same_object(pointer, deallocated_symbol.symbol_expr());
}

exprt null_object(const exprt &pointer)
{
  null_pointer_exprt null_pointer(to_pointer_type(pointer.type()));
  return same_object(null_pointer, pointer);
}

exprt integer_address(const exprt &pointer)
{
  null_pointer_exprt null_pointer(to_pointer_type(pointer.type()));
  return and_exprt(same_object(null_pointer, pointer),
                   notequal_exprt(null_pointer, pointer));
}

exprt object_upper_bound(
  const exprt &pointer,
  const exprt &access_size)
{
  // this is
  // POINTER_OFFSET(p)+access_size>OBJECT_SIZE(pointer)

  exprt object_size_expr=object_size(pointer);

  exprt object_offset=pointer_offset(pointer);

  // need to add size
  irep_idt op=ID_ge;
  exprt sum=object_offset;

  if(access_size.is_not_nil())
  {
    op=ID_gt;

    sum = plus_exprt(
      typecast_exprt::conditional_cast(object_offset, access_size.type()),
      access_size);
  }

  return binary_relation_exprt(
    typecast_exprt::conditional_cast(sum, object_size_expr.type()),
    op,
    object_size_expr);
}

exprt object_lower_bound(
  const exprt &pointer,
  const exprt &offset)
{
  exprt p_offset=pointer_offset(pointer);

  exprt zero=from_integer(0, p_offset.type());

  if(offset.is_not_nil())
  {
    p_offset = plus_exprt(
      p_offset, typecast_exprt::conditional_cast(offset, p_offset.type()));
  }

  return binary_relation_exprt(std::move(p_offset), ID_lt, std::move(zero));
}
