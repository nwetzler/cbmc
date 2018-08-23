/*******************************************************************\

Module: Data structures representing statements in a program

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/


#ifndef CPROVER_UTIL_STD_CODE_H
#define CPROVER_UTIL_STD_CODE_H

#include <list>

#include "expr.h"
#include "expr_cast.h"
#include "invariant.h"
#include "std_expr.h"
#include "std_types.h"
#include "validate.h"
#include "validate_code.h"

/// Data structure for representing an arbitrary statement in a program. Every
/// specific type of statement (e.g. block of statements, assignment,
/// if-then-else statement...) is represented by a subtype of `codet`.
/// `codet`s are represented to be subtypes of \ref exprt since statements can
/// occur in an expression context in C: for example, the assignment `x = y;`
/// is an expression with return value `y`. For other types of statements in an
/// expression context, see e.g.
/// https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html.
/// To distinguish a `codet` from other [exprts](\ref exprt), we set its
/// [id()](\ref irept::id) to `ID_code`. To distinguish different types of
/// `codet`, we use a named sub `ID_statement`.
class codet : public irept
{
public:
  DEPRECATED("Use codet(statement) instead")
  codet() : irept(ID_code)
  {
  }

  /// \param statement: Specifies the type of the `codet` to be constructed,
  ///   e.g. `ID_block` for a \ref code_blockt or `ID_assign` for a
  ///   \ref code_assignt.
  explicit codet(const irep_idt &statement) : irept(ID_code)
  {
    set_statement(statement);
  }

  codet(const irep_idt &statement, source_locationt loc) : irept(ID_code)
  {
    set_statement(statement);
    add_source_location() = std::move(loc);
  }

  /// \param statement: Specifies the type of the `codet` to be constructed,
  ///   e.g. `ID_block` for a \ref code_blockt or `ID_assign` for a
  ///   \ref code_assignt.
  /// \param _op: any operands to be added
  explicit codet(const irep_idt &statement, irept::subt _sub) : codet(statement)
  {
    get_sub() = std::move(_sub);
  }

  codet(const irep_idt &statement, irept::subt _sub, source_locationt loc)
    : codet(statement, std::move(loc))
  {
    get_sub() = std::move(_sub);
  }

  void set_statement(const irep_idt &statement)
  {
    set(ID_statement, statement);
  }

  const irep_idt &get_statement() const
  {
    return get(ID_statement);
  }

  codet &first_statement();
  const codet &first_statement() const;
  codet &last_statement();
  const codet &last_statement() const;

  DEPRECATED(SINCE(2019, 2, 6, "use code_blockt(...) instead"))
  class code_blockt &make_block();

  /// Check that the code statement is well-formed (shallow checks only, i.e.,
  /// enclosed statements, subexpressions, etc. are not checked)
  ///
  /// Subclasses may override this function to provide specific well-formedness
  /// checks for the corresponding types.
  ///
  /// The validation mode indicates whether well-formedness check failures are
  /// reported via DATA_INVARIANT violations or exceptions.
  static void check(const codet &, const validation_modet)
  {
  }

  /// Check that the code statement is well-formed, assuming that all its
  /// enclosed statements, subexpressions, etc. have all ready been checked for
  /// well-formedness.
  ///
  /// Subclasses may override this function to provide specific well-formedness
  /// checks for the corresponding types.
  ///
  /// The validation mode indicates whether well-formedness check failures are
  /// reported via DATA_INVARIANT violations or exceptions.
  static void validate(
    const codet &code,
    const namespacet &,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    check_code(code, vm);
  }

  /// Check that the code statement is well-formed (full check, including checks
  /// of all subexpressions)
  ///
  /// Subclasses may override this function to provide specific well-formedness
  /// checks for the corresponding types.
  ///
  /// The validation mode indicates whether well-formedness check failures are
  /// reported via DATA_INVARIANT violations or exceptions.
  static void validate_full(
    const codet &code,
    const namespacet &,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    check_code(code, vm);
  }

  const source_locationt &find_source_location() const;

  const source_locationt &source_location() const
  {
    return static_cast<const source_locationt &>(find(ID_C_source_location));
  }

  source_locationt &add_source_location()
  {
    return static_cast<source_locationt &>(add(ID_C_source_location));
  }

  const exprt &as_expr() const
  {
    return *static_cast<const exprt *>(static_cast<const irept *>(this));
  }

public:
  // will rename to expr_operandst
  typedef std::vector<exprt> operandst;

  // returns true if there is at least one operand
  bool has_operands() const
  {
    return !operands().empty();
  }

  operandst &operands()
  {
    return (operandst &)get_sub();
  }

  const operandst &operands() const
  {
    return (const operandst &)get_sub();
  }

  exprt &op0()
  {
    return operands().front();
  }

  exprt &op1()
  {
    return operands()[1];
  }

  exprt &op2()
  {
    return operands()[2];
  }

  exprt &op3()
  {
    return operands()[3];
  }

  const exprt &op0() const
  {
    return operands().front();
  }

  const exprt &op1() const
  {
    return operands()[1];
  }

  const exprt &op2() const
  {
    return operands()[2];
  }

  const exprt &op3() const
  {
    return operands()[3];
  }

  void reserve_operands(operandst::size_type n)
  {
    operands().reserve(n);
  }

  // destroys expr, e1, e2, e3
  void move_to_operands(exprt &);
  void move_to_operands(exprt &, exprt &);
  void move_to_operands(exprt &, exprt &, exprt &);
  // does not destroy expr, e1, e2, e3
  void copy_to_operands(const exprt &);
  void copy_to_operands(const exprt &, const exprt &);
  void copy_to_operands(const exprt &, const exprt &, const exprt &);
  void add_to_operands(const exprt &);
  void add_to_operands(const exprt &, const exprt &);
  void add_to_operands(const exprt &, const exprt &, const exprt &);

protected:
  typedef std::vector<codet> code_operandst;

  code_operandst &code_operands()
  { return (code_operandst &)get_sub(); }

  const code_operandst &code_operands() const
  { return (const code_operandst &)get_sub(); }

  codet &code_op0()
  { return code_operands().front(); }

  codet &code_op1()
  { return code_operands()[1]; }

  codet &code_op2()
  { return code_operands()[2]; }

  codet &code_op3()
  { return code_operands()[3]; }

  const codet &code_op0() const
  { return code_operands().front(); }

  const codet &code_op1() const
  { return code_operands()[1]; }

  const codet &code_op2() const
  { return code_operands()[2]; }

  const codet &code_op3() const
  { return code_operands()[3]; }

  // destroys expr, e1, e2, e3
  void move_to_operands(codet &);
  void move_to_operands(codet &, codet &);
  void move_to_operands(codet &, codet &, codet &);
  // does not destroy expr, e1, e2, e3
  void copy_to_operands(const codet &);
  void copy_to_operands(const codet &, const codet &);
  void copy_to_operands(const codet &, const codet &, const codet &);

public:
  void add_to_operands(const codet &);
  void add_to_operands(const codet &, const codet &);
  void add_to_operands(const codet &, const codet &, const codet &);
};

namespace detail // NOLINT
{
template <typename Tag>
inline bool can_cast_code_impl(const codet &code, const Tag &tag)
{
  return code.get_statement() == tag;
}

} // namespace detail

template <typename T>
inline bool can_cast_code(const codet &base);

// to_code has no validation other than checking the id(), so no validate_expr
// is provided for codet

inline const codet &to_code(const irept &irep)
{
  PRECONDITION(irep.id() == ID_code);
  return static_cast<const codet &>(irep);
}

inline codet &to_code(irept &irep)
{
  PRECONDITION(irep.id() == ID_code);
  return static_cast<codet &>(irep);
}

/// A \ref codet representing sequential composition of program statements.
/// Each operand represents a statement in the block.
class code_blockt:public codet
{
public:
  code_blockt():codet(ID_block)
  {
  }

  typedef std::vector<codet> code_operandst;

  code_operandst &statements()
  {
    return (code_operandst &)get_sub();
  }

  const code_operandst &statements() const
  {
    return (const code_operandst &)get_sub();
  }

  static code_blockt from_list(const std::list<codet> &_list)
  {
    code_blockt result;
    auto &o = result.statements();
    o.reserve(_list.size());
    for(const auto &statement : _list)
      o.push_back(statement);
    return result;
  }

  explicit code_blockt(const std::vector<codet> &_statements)
    : codet(ID_block, (const irept::subt &)_statements)
  {
  }

  explicit code_blockt(std::vector<codet> &&_statements)
    : codet(ID_block, std::move((irept::subt &&) _statements))
  {
  }

  void add(const codet &code)
  {
    add_to_operands(code);
  }

  void add(codet &&code)
  {
    add_to_operands(std::move(code));
  }

  void add(codet code, source_locationt loc)
  {
    code.add_source_location().swap(loc);
    add(std::move(code));
  }

  void append(const code_blockt &extra_block);

  // This is the closing '}' or 'END' at the end of a block
  source_locationt end_location() const
  {
    return static_cast<const source_locationt &>(find(ID_C_end_location));
  }

  codet &find_last_statement();

  static void validate_full(
    const codet &code,
    const namespacet &ns,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    for(const auto &statement : code.operands())
    {
      DATA_CHECK(
        vm, code.id() == ID_code, "code block must be made up of codet");
      validate_full_code(to_code(statement), ns, vm);
    }
  }
};

template <>
inline bool can_cast_code<code_blockt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_block);
}

// to_code_block has no validation other than checking the statement(), so no
// validate_expr is provided for code_blockt

inline const code_blockt &to_code_block(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_block);
  return static_cast<const code_blockt &>(code);
}

inline code_blockt &to_code_block(codet &code)
{
  PRECONDITION(code.get_statement() == ID_block);
  return static_cast<code_blockt &>(code);
}

/// A \ref codet representing a `skip` statement.
class code_skipt:public codet
{
public:
  code_skipt():codet(ID_skip)
  {
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_skipt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_skip);
}

// there is no to_code_skip, so no validate_expr is provided for code_skipt

/// A \ref codet representing an assignment in the program.
/// For example, if an expression `e1` is represented as an \ref exprt `expr1`
/// and an expression `e2` is represented as an \ref exprt `expr2`, the
/// assignment `e1 = e2;` can be represented as `code_assignt(expr1, expr2)`.
class code_assignt:public codet
{
public:
  code_assignt():codet(ID_assign)
  {
    operands().resize(2);
  }

  code_assignt(exprt lhs, exprt rhs)
    : codet(ID_assign, {std::move(lhs), std::move(rhs)})
  {
  }

  code_assignt(exprt lhs, exprt rhs, source_locationt loc)
    : codet(ID_assign, {std::move(lhs), std::move(rhs)}, std::move(loc))
  {
  }

  exprt &lhs()
  {
    return op0();
  }

  exprt &rhs()
  {
    return op1();
  }

  const exprt &lhs() const
  {
    return op0();
  }

  const exprt &rhs() const
  {
    return op1();
  }

  static void check(
    const codet &code,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    DATA_CHECK(
      vm, code.operands().size() == 2, "assignment must have two operands");
  }

  static void validate(
    const codet &code,
    const namespacet &,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    check(code, vm);

    DATA_CHECK(
      vm,
      code.op0().type() == code.op1().type(),
      "lhs and rhs of assignment must have same type");
  }

  static void validate_full(
    const codet &code,
    const namespacet &ns,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    for(const exprt &op : code.operands())
    {
      validate_full_expr(op, ns, vm);
    }

    validate(code, ns, vm);
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

inline void validate_operands(
  const codet &value,
  exprt::operandst::size_type number,
  const char *message,
  bool allow_more = false)
{
  DATA_INVARIANT(
    allow_more ? value.operands().size() >= number
               : value.operands().size() == number,
    message);
}

template <>
inline bool can_cast_code<code_assignt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_assign);
}

inline void validate_expr(const code_assignt & x)
{
  code_assignt::check(x);
}

inline const code_assignt &to_code_assign(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_assign);
  code_assignt::check(code);
  return static_cast<const code_assignt &>(code);
}

inline code_assignt &to_code_assign(codet &code)
{
  PRECONDITION(code.get_statement() == ID_assign);
  code_assignt::check(code);
  return static_cast<code_assignt &>(code);
}

/// A `codet` representing the declaration of a local variable.
/// For example, if a variable (symbol) `x` is represented as a
/// \ref symbol_exprt `sym`, then the declaration of this variable can be
/// represented as `code_declt(sym)`.
class code_declt:public codet
{
public:
  explicit code_declt(symbol_exprt symbol) : codet(ID_decl, {std::move(symbol)})
  {
  }

  symbol_exprt &symbol()
  {
    return static_cast<symbol_exprt &>(op0());
  }

  const symbol_exprt &symbol() const
  {
    return static_cast<const symbol_exprt &>(op0());
  }

  const irep_idt &get_identifier() const
  {
    return symbol().get_identifier();
  }

  static void check(
    const codet &code,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    // will be size()==1 in the future
    DATA_CHECK(
      vm,
      code.operands().size() >= 1,
      "declaration must have one or more operands");
    DATA_CHECK(
      vm,
      code.op0().id() == ID_symbol,
      "declaring a non-symbol: " +
        id2string(to_symbol_expr(code.op0()).get_identifier()));
  }
};

template <>
inline bool can_cast_code<code_declt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_decl);
}

inline void validate_expr(const code_declt &x)
{
  code_declt::check(x);
}

inline const code_declt &to_code_decl(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_decl);
  code_declt::check(code);
  return static_cast<const code_declt &>(code);
}

inline code_declt &to_code_decl(codet &code)
{
  PRECONDITION(code.get_statement() == ID_decl);
  code_declt::check(code);
  return static_cast<code_declt &>(code);
}

/// A \ref codet representing the removal of a local variable going out of
/// scope.
class code_deadt:public codet
{
public:
  explicit code_deadt(symbol_exprt symbol) : codet(ID_dead, {std::move(symbol)})
  {
  }

  symbol_exprt &symbol()
  {
    return static_cast<symbol_exprt &>(op0());
  }

  const symbol_exprt &symbol() const
  {
    return static_cast<const symbol_exprt &>(op0());
  }

  const irep_idt &get_identifier() const
  {
    return symbol().get_identifier();
  }

  static void check(
    const codet &code,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    DATA_CHECK(
      vm,
      code.operands().size() == 1,
      "removal (code_deadt) must have one operand");
    DATA_CHECK(
      vm,
      code.op0().id() == ID_symbol,
      "removing a non-symbol: " + id2string(code.op0().id()) + "from scope");
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_deadt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_dead);
}

inline void validate_expr(const code_deadt &x)
{
  code_deadt::check(x);
}

inline const code_deadt &to_code_dead(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_dead);
  code_deadt::check(code);
  return static_cast<const code_deadt &>(code);
}

inline code_deadt &to_code_dead(codet &code)
{
  PRECONDITION(code.get_statement() == ID_dead);
  code_deadt::check(code);
  return static_cast<code_deadt &>(code);
}

/// An assumption, which must hold in subsequent code.
class code_assumet:public codet
{
public:
  explicit code_assumet(exprt expr) : codet(ID_assume, {std::move(expr)})
  {
  }

  const exprt &assumption() const
  {
    return op0();
  }

  exprt &assumption()
  {
    return op0();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_assumet>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_assume);
}

inline void validate_expr(const code_assumet &x)
{
  validate_operands(x, 1, "assume must have one operand");
}

inline const code_assumet &to_code_assume(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_assume);
  const code_assumet &ret = static_cast<const code_assumet &>(code);
  validate_expr(ret);
  return ret;
}

inline code_assumet &to_code_assume(codet &code)
{
  PRECONDITION(code.get_statement() == ID_assume);
  code_assumet &ret = static_cast<code_assumet &>(code);
  validate_expr(ret);
  return ret;
}

/// A non-fatal assertion, which checks a condition then permits execution to
/// continue.
class code_assertt:public codet
{
public:
  explicit code_assertt(exprt expr) : codet(ID_assert, {std::move(expr)})
  {
  }

  const exprt &assertion() const
  {
    return op0();
  }

  exprt &assertion()
  {
    return op0();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_assertt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_assert);
}

inline void validate_expr(const code_assertt &x)
{
  validate_operands(x, 1, "assert must have one operand");
}

inline const code_assertt &to_code_assert(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_assert);
  const code_assertt &ret = static_cast<const code_assertt &>(code);
  validate_expr(ret);
  return ret;
}

inline code_assertt &to_code_assert(codet &code)
{
  PRECONDITION(code.get_statement() == ID_assert);
  code_assertt &ret = static_cast<code_assertt &>(code);
  validate_expr(ret);
  return ret;
}

/// A `codet` representing the declaration that an input of a particular
/// description has a value which corresponds to the value of a given expression
/// (or expressions).
/// When working with the C front end, calls to the `__CPROVER_input` intrinsic
/// can be added to the input code in order add instructions of this type to the
/// goto program.
/// The first argument is expected to be a C string denoting the input
/// identifier. The second argument is the expression for the input value.
class code_inputt : public codet
{
public:
  /// This constructor is for support of calls to `__CPROVER_input` in user
  /// code. Where the first first argument is a description which may be any
  /// `const char *` and one or more corresponding expression arguments follow.
  explicit code_inputt(
    std::vector<exprt> arguments,
    optionalt<source_locationt> location = {});

  /// This constructor is intended for generating input instructions as part of
  /// synthetic entry point code, rather than as part of user code.
  /// \param description: This is used to construct an expression for a pointer
  ///   to a string constant containing the description text. This expression
  ///   is then used as the first argument.
  /// \param expression: This expression corresponds to a value which should be
  ///   recorded as an input.
  /// \param location: A location to associate with this instruction.
  code_inputt(
    const irep_idt &description,
    exprt expression,
    optionalt<source_locationt> location = {});

  static void check(
    const codet &code,
    const validation_modet vm = validation_modet::INVARIANT);
};

#if 0
template <>
inline bool can_cast_expr<code_inputt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_input);
}
#endif

inline void validate_expr(const code_inputt &input)
{
  code_inputt::check(input);
}

/// A `codet` representing the declaration that an output of a particular
/// description has a value which corresponds to the value of a given expression
/// (or expressions).
/// When working with the C front end, calls to the `__CPROVER_output` intrinsic
/// can be added to the input code in order add instructions of this type to the
/// goto program.
/// The first argument is expected to be a C string denoting the output
/// identifier. The second argument is the expression for the output value.
class code_outputt : public codet
{
public:
  /// This constructor is for support of calls to `__CPROVER_output` in user
  /// code. Where the first first argument is a description which may be any
  /// `const char *` and one or more corresponding expression arguments follow.
  explicit code_outputt(
    std::vector<exprt> arguments,
    optionalt<source_locationt> location = {});

  /// This constructor is intended for generating output instructions as part of
  /// synthetic entry point code, rather than as part of user code.
  /// \param description: This is used to construct an expression for a pointer
  ///   to a string constant containing the description text.
  /// \param expression: This expression corresponds to a value which should be
  ///   recorded as an output.
  /// \param location: A location to associate with this instruction.
  code_outputt(
    const irep_idt &description,
    exprt expression,
    optionalt<source_locationt> location = {});

  static void check(
    const codet &code,
    const validation_modet vm = validation_modet::INVARIANT);
};

#if 0
template <>
inline bool can_cast_expr<code_outputt>(const exprt &base)
{
  return detail::can_cast_code_impl(base, ID_output);
}
#endif

inline void validate_expr(const code_outputt &output)
{
  code_outputt::check(output);
}

/// Create a fatal assertion, which checks a condition and then halts if it does
/// not hold. Equivalent to `ASSERT(condition); ASSUME(condition)`.
///
/// Source level assertions should probably use this, whilst checks that are
/// normally non-fatal at runtime, such as integer overflows, should use
/// code_assertt by itself.
/// \param condition: condition to assert
/// \param source_location: source location to attach to the generated code;
///   conventionally this should have `comment` and `property_class` fields set
///   to indicate the nature of the assertion.
/// \return A code block that asserts a condition then aborts if it does not
///   hold.
code_blockt create_fatal_assertion(
  const exprt &condition, const source_locationt &source_location);

/// \ref codet representation of an if-then-else statement.
class code_ifthenelset:public codet
{
public:
  DEPRECATED(SINCE(
    2018,
    12,
    2,
    "use code_ifthenelset(condition, then_code[, else_code]) instead"))
  code_ifthenelset():codet(ID_ifthenelse)
  {
    operands().resize(3);
    op1().make_nil();
    op2().make_nil();
  }

  /// An if \p condition then \p then_code else \p else_code statement.
  code_ifthenelset(exprt condition, codet then_code, codet else_code)
    : codet(
        ID_ifthenelse,
        {std::move(condition), std::move(then_code), std::move(else_code)})
  {
  }

  /// An if \p condition then \p then_code statement (no "else" case).
  code_ifthenelset(exprt condition, codet then_code)
    : codet(
        ID_ifthenelse,
        {std::move(condition), std::move(then_code), nil_exprt()})
  {
  }

  const exprt &cond() const
  {
    return op0();
  }

  exprt &cond()
  {
    return op0();
  }

  const codet &then_case() const
  {
    return code_op1();
  }

  bool has_else_case() const
  {
    return op2().is_not_nil();
  }

  const codet &else_case() const
  {
    return code_op2();
  }

  codet &then_case()
  {
    return code_op1();
  }

  codet &else_case()
  {
    return code_op2();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_ifthenelset>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_ifthenelse);
}

inline void validate_expr(const code_ifthenelset &x)
{
  validate_operands(x, 3, "if-then-else must have three operands");
}

inline const code_ifthenelset &to_code_ifthenelse(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_ifthenelse);
  const code_ifthenelset &ret = static_cast<const code_ifthenelset &>(code);
  validate_expr(ret);
  return ret;
}

inline code_ifthenelset &to_code_ifthenelse(codet &code)
{
  PRECONDITION(code.get_statement() == ID_ifthenelse);
  code_ifthenelset &ret = static_cast<code_ifthenelset &>(code);
  validate_expr(ret);
  return ret;
}

/// \ref codet representing a `switch` statement.
class code_switcht:public codet
{
public:
  code_switcht(exprt _value, codet _body)
    : codet(ID_switch, {std::move(_value), std::move(_body)})
  {
  }

  const exprt &value() const
  {
    return op0();
  }

  exprt &value()
  {
    return op0();
  }

  const codet &body() const
  {
    return code_op1();
  }

  codet &body()
  {
    return code_op1();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_switcht>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_switch);
}

inline void validate_expr(const code_switcht &x)
{
  validate_operands(x, 2, "switch must have two operands");
}

inline const code_switcht &to_code_switch(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_switch);
  const code_switcht &ret = static_cast<const code_switcht &>(code);
  validate_expr(ret);
  return ret;
}

inline code_switcht &to_code_switch(codet &code)
{
  PRECONDITION(code.get_statement() == ID_switch);
  code_switcht &ret = static_cast<code_switcht &>(code);
  validate_expr(ret);
  return ret;
}

/// \ref codet representing a `while` statement.
class code_whilet:public codet
{
public:
  code_whilet(exprt _cond, codet _body)
    : codet(ID_while, {std::move(_cond), std::move(_body)})
  {
  }

  const exprt &cond() const
  {
    return op0();
  }

  exprt &cond()
  {
    return op0();
  }

  const codet &body() const
  {
    return code_op1();
  }

  codet &body()
  {
    return code_op1();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_whilet>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_while);
}

inline void validate_expr(const code_whilet &x)
{
  validate_operands(x, 2, "while must have two operands");
}

inline const code_whilet &to_code_while(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_while);
  const code_whilet &ret = static_cast<const code_whilet &>(code);
  validate_expr(ret);
  return ret;
}

inline code_whilet &to_code_while(codet &code)
{
  PRECONDITION(code.get_statement() == ID_while);
  code_whilet &ret = static_cast<code_whilet &>(code);
  validate_expr(ret);
  return ret;
}

/// \ref codet representation of a `do while` statement.
class code_dowhilet:public codet
{
public:
  code_dowhilet(exprt _cond, codet _body)
    : codet(ID_dowhile, {std::move(_cond), std::move(_body)})
  {
  }

  const exprt &cond() const
  {
    return op0();
  }

  exprt &cond()
  {
    return op0();
  }

  const codet &body() const
  {
    return code_op1();
  }

  codet &body()
  {
    return code_op1();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_dowhilet>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_dowhile);
}

inline void validate_expr(const code_dowhilet &x)
{
  validate_operands(x, 2, "do-while must have two operands");
}

inline const code_dowhilet &to_code_dowhile(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_dowhile);
  const code_dowhilet &ret = static_cast<const code_dowhilet &>(code);
  validate_expr(ret);
  return ret;
}

inline code_dowhilet &to_code_dowhile(codet &code)
{
  PRECONDITION(code.get_statement() == ID_dowhile);
  code_dowhilet &ret = static_cast<code_dowhilet &>(code);
  validate_expr(ret);
  return ret;
}

/// \ref codet representation of a `for` statement.
class code_fort:public codet
{
public:
  DEPRECATED(
    SINCE(2018, 12, 2, "use code_fort(init, cond, iter, body) instead"))
  code_fort():codet(ID_for)
  {
    operands().resize(4);
  }

  /// A statement describing a for loop with initializer \p _init, loop
  /// condition \p _cond, increment \p _iter, and body \p _body.
  code_fort(exprt _init, exprt _cond, exprt _iter, codet _body)
    : codet(
        ID_for,
        {std::move(_init),
         std::move(_cond),
         std::move(_iter),
         std::move(_body)})
  {
  }

  // nil or a statement
  const exprt &init() const
  {
    return op0();
  }

  exprt &init()
  {
    return op0();
  }

  const exprt &cond() const
  {
    return op1();
  }

  exprt &cond()
  {
    return op1();
  }

  const exprt &iter() const
  {
    return op2();
  }

  exprt &iter()
  {
    return op2();
  }

  const codet &body() const
  {
    return code_op3();
  }

  codet &body()
  {
    return code_op3();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_fort>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_for);
}

inline void validate_expr(const code_fort &x)
{
  validate_operands(x, 4, "for must have four operands");
}

inline const code_fort &to_code_for(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_for);
  const code_fort &ret = static_cast<const code_fort &>(code);
  validate_expr(ret);
  return ret;
}

inline code_fort &to_code_for(codet &code)
{
  PRECONDITION(code.get_statement() == ID_for);
  code_fort &ret = static_cast<code_fort &>(code);
  validate_expr(ret);
  return ret;
}

/// \ref codet representation of a `goto` statement.
class code_gotot:public codet
{
public:
  explicit code_gotot(const irep_idt &label):codet(ID_goto)
  {
    set_destination(label);
  }

  void set_destination(const irep_idt &label)
  {
    set(ID_destination, label);
  }

  const irep_idt &get_destination() const
  {
    return get(ID_destination);
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_gotot>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_goto);
}

inline void validate_expr(const code_gotot &x)
{
  validate_operands(x, 0, "goto must not have operands");
}

inline const code_gotot &to_code_goto(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_goto);
  const code_gotot &ret = static_cast<const code_gotot &>(code);
  validate_expr(ret);
  return ret;
}

inline code_gotot &to_code_goto(codet &code)
{
  PRECONDITION(code.get_statement() == ID_goto);
  code_gotot &ret = static_cast<code_gotot &>(code);
  validate_expr(ret);
  return ret;
}

/// \ref codet representation of a function call statement.
/// The function call statement has three operands.
/// The first is the expression that is used to store the return value.
/// The second is the function called.
/// The third is a vector of argument values.
class code_function_callt:public codet
{
public:
  explicit code_function_callt(exprt _function)
    : codet(
        ID_function_call,
        {nil_exprt(), std::move(_function), exprt(ID_arguments)})
  {
  }

  typedef exprt::operandst argumentst;

  code_function_callt(exprt _lhs, exprt _function, argumentst _arguments)
    : codet(
        ID_function_call,
        {std::move(_lhs), std::move(_function), exprt(ID_arguments)})
  {
    arguments() = std::move(_arguments);
  }

  code_function_callt(exprt _function, argumentst _arguments)
    : code_function_callt(std::move(_function))
  {
    arguments() = std::move(_arguments);
  }

  exprt &lhs()
  {
    return op0();
  }

  const exprt &lhs() const
  {
    return op0();
  }

  exprt &function()
  {
    return op1();
  }

  const exprt &function() const
  {
    return op1();
  }

  argumentst &arguments()
  {
    return op2().operands();
  }

  const argumentst &arguments() const
  {
    return op2().operands();
  }

  static void check(
    const codet &code,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    DATA_CHECK(
      vm,
      code.operands().size() == 3,
      "function calls must have three operands:\n1) expression to store the "
      "returned values\n2) the function being called\n3) the vector of "
      "arguments");
  }

  static void validate(
    const codet &code,
    const namespacet &,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    check(code, vm);

    if(code.op0().id() != ID_nil)
      DATA_CHECK(
        vm,
        code.op0().type() == to_code_type(code.op1().type()).return_type(),
        "function returns expression of wrong type");
  }

  static void validate_full(
    const codet &code,
    const namespacet &ns,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    for(const exprt &op : code.operands())
    {
      validate_full_expr(op, ns, vm);
    }

    validate(code, ns, vm);
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_function_callt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_function_call);
}

inline void validate_expr(const code_function_callt &x)
{
  code_function_callt::check(x);
}

inline const code_function_callt &to_code_function_call(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_function_call);
  code_function_callt::check(code);
  return static_cast<const code_function_callt &>(code);
}

inline code_function_callt &to_code_function_call(codet &code)
{
  PRECONDITION(code.get_statement() == ID_function_call);
  code_function_callt::check(code);
  return static_cast<code_function_callt &>(code);
}

/// \ref codet representation of a "return from a function" statement.
class code_returnt:public codet
{
public:
  code_returnt() : codet(ID_return, {nil_exprt()})
  {
  }

  explicit code_returnt(exprt _op) : codet(ID_return, {std::move(_op)})
  {
  }

  const exprt &return_value() const
  {
    return op0();
  }

  exprt &return_value()
  {
    return op0();
  }

  bool has_return_value() const
  {
    return return_value().is_not_nil();
  }

  static void check(
    const codet &code,
    const validation_modet vm = validation_modet::INVARIANT)
  {
    DATA_CHECK(vm, code.operands().size() == 1, "return must have one operand");
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_returnt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_return);
}

inline void validate_expr(const code_returnt &x)
{
  code_returnt::check(x);
}

inline const code_returnt &to_code_return(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_return);
  code_returnt::check(code);
  return static_cast<const code_returnt &>(code);
}

inline code_returnt &to_code_return(codet &code)
{
  PRECONDITION(code.get_statement() == ID_return);
  code_returnt::check(code);
  return static_cast<code_returnt &>(code);
}

/// \ref codet representation of a label for branch targets.
class code_labelt:public codet
{
public:
  DEPRECATED(SINCE(2019, 2, 6, "use code_labelt(label, _code) instead"))
  explicit code_labelt(const irep_idt &_label):codet(ID_label)
  {
    operands().resize(1);
    set_label(_label);
  }

  code_labelt(const irep_idt &_label, codet _code)
    : codet(ID_label, {std::move(_code)})
  {
    set_label(_label);
  }

  const irep_idt &get_label() const
  {
    return get(ID_label);
  }

  void set_label(const irep_idt &label)
  {
    set(ID_label, label);
  }

  codet &code()
  {
    return code_op0();
  }

  const codet &code() const
  {
    return code_op0();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_labelt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_label);
}

inline void validate_expr(const code_labelt &x)
{
  validate_operands(x, 1, "label must have one operand");
}

inline const code_labelt &to_code_label(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_label);
  const code_labelt &ret = static_cast<const code_labelt &>(code);
  validate_expr(ret);
  return ret;
}

inline code_labelt &to_code_label(codet &code)
{
  PRECONDITION(code.get_statement() == ID_label);
  code_labelt &ret = static_cast<code_labelt &>(code);
  validate_expr(ret);
  return ret;
}

/// \ref codet representation of a switch-case, i.e.\ a `case` statement within
/// a `switch`.
class code_switch_caset:public codet
{
public:
  code_switch_caset(exprt _case_op, codet _code)
    : codet(ID_switch_case, {std::move(_case_op), std::move(_code)})
  {
  }

  bool is_default() const
  {
    return get_bool(ID_default);
  }

  void set_default()
  {
    return set(ID_default, true);
  }

  const exprt &case_op() const
  {
    return op0();
  }

  exprt &case_op()
  {
    return op0();
  }

  codet &code()
  {
    return code_op1();
  }

  const codet &code() const
  {
    return code_op1();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_switch_caset>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_switch_case);
}

inline void validate_expr(const code_switch_caset &x)
{
  validate_operands(x, 2, "switch-case must have two operands");
}

inline const code_switch_caset &to_code_switch_case(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_switch_case);
  const code_switch_caset &ret = static_cast<const code_switch_caset &>(code);
  validate_expr(ret);
  return ret;
}

inline code_switch_caset &to_code_switch_case(codet &code)
{
  PRECONDITION(code.get_statement() == ID_switch_case);
  code_switch_caset &ret = static_cast<code_switch_caset &>(code);
  validate_expr(ret);
  return ret;
}

/// \ref codet representation of a switch-case, i.e.\ a `case` statement
/// within a `switch`. This is the variant that takes a range,
/// which is a gcc extension.
class code_gcc_switch_case_ranget : public codet
{
public:
  code_gcc_switch_case_ranget(exprt _lower, exprt _upper, codet _code)
    : codet(
        ID_gcc_switch_case_range,
        {std::move(_lower), std::move(_upper), std::move(_code)})
  {
  }

  /// lower bound of range
  const exprt &lower() const
  {
    return op0();
  }

  /// lower bound of range
  exprt &lower()
  {
    return op0();
  }

  /// upper bound of range
  const exprt &upper() const
  {
    return op1();
  }

  /// upper bound of range
  exprt &upper()
  {
    return op1();
  }

  /// the statement to be executed when the case applies
  codet &code()
  {
    return static_cast<codet &>(get_sub()[2]);
  }

  /// the statement to be executed when the case applies
  const codet &code() const
  {
    return static_cast<const codet &>(get_sub()[2]);
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

inline const code_gcc_switch_case_ranget &
to_code_gcc_switch_case_range(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_gcc_switch_case_range);
  const code_gcc_switch_case_ranget &ret =
    static_cast<const code_gcc_switch_case_ranget &>(code);
  //validate_expr(ret);
  return ret;
}

inline code_gcc_switch_case_ranget &to_code_gcc_switch_case_range(codet &code)
{
  PRECONDITION(code.get_statement() == ID_gcc_switch_case_range);
  code_gcc_switch_case_ranget &ret =
    static_cast<code_gcc_switch_case_ranget &>(code);
  //validate_expr(ret);
  return ret;
}

/// \ref codet representation of a `break` statement (within a `for` or `while`
/// loop).
class code_breakt:public codet
{
public:
  code_breakt():codet(ID_break)
  {
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_breakt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_break);
}

// to_code_break only checks the code statement, so no validate_expr is
// provided for code_breakt

inline const code_breakt &to_code_break(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_break);
  return static_cast<const code_breakt &>(code);
}

inline code_breakt &to_code_break(codet &code)
{
  PRECONDITION(code.get_statement() == ID_break);
  return static_cast<code_breakt &>(code);
}

/// \ref codet representation of a `continue` statement (within a `for` or
/// `while` loop).
class code_continuet:public codet
{
public:
  code_continuet():codet(ID_continue)
  {
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_continuet>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_continue);
}

// to_code_continue only checks the code statement, so no validate_expr is
// provided for code_continuet

inline const code_continuet &to_code_continue(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_continue);
  return static_cast<const code_continuet &>(code);
}

inline code_continuet &to_code_continue(codet &code)
{
  PRECONDITION(code.get_statement() == ID_continue);
  return static_cast<code_continuet &>(code);
}

/// \ref codet representation of an inline assembler statement.
class code_asmt:public codet
{
public:
  code_asmt():codet(ID_asm)
  {
  }

  explicit code_asmt(exprt expr) : codet(ID_asm, {std::move(expr)})
  {
  }

  const irep_idt &get_flavor() const
  {
    return get(ID_flavor);
  }

  void set_flavor(const irep_idt &f)
  {
    set(ID_flavor, f);
  }
};

template <>
inline bool can_cast_code<code_asmt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_asm);
}

// to_code_asm only checks the code statement, so no validate_expr is
// provided for code_asmt

inline code_asmt &to_code_asm(codet &code)
{
  PRECONDITION(code.get_statement() == ID_asm);
  return static_cast<code_asmt &>(code);
}

inline const code_asmt &to_code_asm(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_asm);
  return static_cast<const code_asmt &>(code);
}

/// \ref codet representation of an inline assembler statement,
/// for the gcc flavor.
class code_asm_gcct : public code_asmt
{
public:
  code_asm_gcct()
  {
    set_flavor(ID_gcc);
    operands().resize(5);
  }

  exprt &asm_text()
  {
    return op0();
  }

  const exprt &asm_text() const
  {
    return op0();
  }

  exprt &outputs()
  {
    return op1();
  }

  const exprt &outputs() const
  {
    return op1();
  }

  exprt &inputs()
  {
    return op2();
  }

  const exprt &inputs() const
  {
    return op2();
  }

  exprt &clobbers()
  {
    return op3();
  }

  const exprt &clobbers() const
  {
    return op3();
  }

  exprt &labels()
  {
    return operands()[4];
  }

  const exprt &labels() const
  {
    return operands()[4];
  }

protected:
  using code_asmt::op0;
  using code_asmt::op1;
  using code_asmt::op2;
  using code_asmt::op3;
};

inline void validate_expr(const code_asm_gcct &x)
{
  validate_operands(x, 5, "code_asm_gcc must have five operands");
}

inline code_asm_gcct &to_code_asm_gcc(codet &code)
{
  PRECONDITION(code.get_statement() == ID_asm);
  PRECONDITION(to_code_asm(code).get_flavor() == ID_gcc);
  code_asm_gcct &ret = static_cast<code_asm_gcct &>(code);
  validate_expr(ret);
  return ret;
}

inline const code_asm_gcct &to_code_asm_gcc(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_asm);
  PRECONDITION(to_code_asm(code).get_flavor() == ID_gcc);
  const code_asm_gcct &ret = static_cast<const code_asm_gcct &>(code);
  validate_expr(ret);
  return ret;
}

/// \ref codet representation of an expression statement.
/// It has one operand, which is the expression it stores.
class code_expressiont:public codet
{
public:
  explicit code_expressiont(exprt expr)
    : codet(ID_expression, {std::move(expr)})
  {
  }

  const exprt &expression() const
  {
    return op0();
  }

  exprt &expression()
  {
    return op0();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_expressiont>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_expression);
}

inline void validate_expr(const code_expressiont &x)
{
  validate_operands(x, 1, "expression statement must have one operand");
}

inline code_expressiont &to_code_expression(codet &code)
{
  PRECONDITION(code.get_statement() == ID_expression);
  code_expressiont &ret = static_cast<code_expressiont &>(code);
  validate_expr(ret);
  return ret;
}

inline const code_expressiont &to_code_expression(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_expression);
  const code_expressiont &ret = static_cast<const code_expressiont &>(code);
  validate_expr(ret);
  return ret;
}

/// An expression containing a side effect.
/// Note that unlike most classes in this file, `side_effect_exprt` and its
/// subtypes are not subtypes of \ref codet, but they inherit directly from
/// \ref exprt. They do have a `statement` like [codets](\ref codet), but their
/// [id()](\ref irept::id) is `ID_side_effect`, not `ID_code`.
class side_effect_exprt : public exprt
{
public:
  DEPRECATED(
    SINCE(2018, 8, 9, "use side_effect_exprt(statement, type, loc) instead"))
  side_effect_exprt(const irep_idt &statement, const typet &_type)
    : exprt(ID_side_effect, _type)
  {
    set_statement(statement);
  }

  /// constructor with operands
  side_effect_exprt(
    const irep_idt &statement,
    operandst _operands,
    typet _type,
    source_locationt loc)
    : exprt(ID_side_effect, std::move(_type), std::move(loc))
  {
    set_statement(statement);
    operands() = std::move(_operands);
  }

  side_effect_exprt(
    const irep_idt &statement,
    typet _type,
    source_locationt loc)
    : exprt(ID_side_effect, std::move(_type), std::move(loc))
  {
    set_statement(statement);
  }

  const irep_idt &get_statement() const
  {
    return get(ID_statement);
  }

  void set_statement(const irep_idt &statement)
  {
    return set(ID_statement, statement);
  }
};

namespace detail // NOLINT
{

template<typename Tag>
inline bool can_cast_side_effect_expr_impl(const exprt &expr, const Tag &tag)
{
  if(const auto ptr = expr_try_dynamic_cast<side_effect_exprt>(expr))
  {
    return ptr->get_statement() == tag;
  }
  return false;
}

} // namespace detail

template<> inline bool can_cast_expr<side_effect_exprt>(const exprt &base)
{
  return base.id()==ID_side_effect;
}

// to_side_effect_expr only checks the id, so no validate_expr is provided for
// side_effect_exprt

inline side_effect_exprt &to_side_effect_expr(exprt &expr)
{
  PRECONDITION(expr.id() == ID_side_effect);
  return static_cast<side_effect_exprt &>(expr);
}

inline const side_effect_exprt &to_side_effect_expr(const exprt &expr)
{
  PRECONDITION(expr.id() == ID_side_effect);
  return static_cast<const side_effect_exprt &>(expr);
}

/// A \ref side_effect_exprt that returns a non-deterministically chosen value.
class side_effect_expr_nondett:public side_effect_exprt
{
public:
  DEPRECATED(SINCE(
    2018,
    8,
    9,
    "use side_effect_expr_nondett(statement, type, loc) instead"))
  explicit side_effect_expr_nondett(const typet &_type):
    side_effect_exprt(ID_nondet, _type)
  {
    set_nullable(true);
  }

  side_effect_expr_nondett(typet _type, source_locationt loc)
    : side_effect_exprt(ID_nondet, std::move(_type), std::move(loc))
  {
    set_nullable(true);
  }

  void set_nullable(bool nullable)
  {
    set(ID_is_nondet_nullable, nullable);
  }

  bool get_nullable() const
  {
    return get_bool(ID_is_nondet_nullable);
  }
};

template<>
inline bool can_cast_expr<side_effect_expr_nondett>(const exprt &base)
{
  return detail::can_cast_side_effect_expr_impl(base, ID_nondet);
}

// to_side_effect_expr_nondet only checks the id, so no validate_expr is
// provided for side_effect_expr_nondett

inline side_effect_expr_nondett &to_side_effect_expr_nondet(exprt &expr)
{
  auto &side_effect_expr_nondet=to_side_effect_expr(expr);
  PRECONDITION(side_effect_expr_nondet.get_statement() == ID_nondet);
  return static_cast<side_effect_expr_nondett &>(side_effect_expr_nondet);
}

inline const side_effect_expr_nondett &to_side_effect_expr_nondet(
  const exprt &expr)
{
  const auto &side_effect_expr_nondet=to_side_effect_expr(expr);
  PRECONDITION(side_effect_expr_nondet.get_statement() == ID_nondet);
  return static_cast<const side_effect_expr_nondett &>(side_effect_expr_nondet);
}

/// A \ref side_effect_exprt that performs an assignment
class side_effect_expr_assignt : public side_effect_exprt
{
public:
  /// construct an assignment side-effect, given lhs and rhs
  /// The type is copied from lhs
  side_effect_expr_assignt(
    const exprt &_lhs,
    const exprt &_rhs,
    const source_locationt &loc)
    : side_effect_exprt(ID_assign, {_lhs, _rhs}, _lhs.type(), loc)
  {
  }

  /// construct an assignment side-effect, given lhs, rhs and the type
  side_effect_expr_assignt(
    exprt _lhs,
    exprt _rhs,
    typet _type,
    source_locationt loc)
    : side_effect_exprt(
        ID_assign,
        {std::move(_lhs), std::move(_rhs)},
        std::move(_type),
        std::move(loc))
  {
  }

  exprt &lhs()
  {
    return op0();
  }

  const exprt &lhs() const
  {
    return op0();
  }

  exprt &rhs()
  {
    return op1();
  }

  const exprt &rhs() const
  {
    return op1();
  }
};

template <>
inline bool can_cast_expr<side_effect_expr_assignt>(const exprt &base)
{
  return detail::can_cast_side_effect_expr_impl(base, ID_assign);
}

inline side_effect_expr_assignt &to_side_effect_expr_assign(exprt &expr)
{
  auto &side_effect_expr_assign = to_side_effect_expr(expr);
  PRECONDITION(side_effect_expr_assign.get_statement() == ID_assign);
  return static_cast<side_effect_expr_assignt &>(side_effect_expr_assign);
}

inline const side_effect_expr_assignt &
to_side_effect_expr_assign(const exprt &expr)
{
  const auto &side_effect_expr_assign = to_side_effect_expr(expr);
  PRECONDITION(side_effect_expr_assign.get_statement() == ID_assign);
  return static_cast<const side_effect_expr_assignt &>(side_effect_expr_assign);
}

/// A \ref side_effect_exprt that contains a statement
class side_effect_expr_statement_expressiont : public side_effect_exprt
{
public:
  /// construct an assignment side-effect, given lhs, rhs and the type
  side_effect_expr_statement_expressiont(
    codet _code,
    typet _type,
    source_locationt loc)
    : side_effect_exprt(
        ID_statement_expression,
        {},
        std::move(_type),
        std::move(loc))
  {
    set(ID_statement, std::move(_code));
  }

  codet &statement()
  {
    return static_cast<codet &>(add(ID_statement));
  }

  const codet &statement() const
  {
    return static_cast<const codet &>(find(ID_statement));
  }
};

template <>
inline bool
can_cast_expr<side_effect_expr_statement_expressiont>(const exprt &base)
{
  return detail::can_cast_side_effect_expr_impl(base, ID_statement_expression);
}

inline side_effect_expr_statement_expressiont &
to_side_effect_expr_statement_expression(exprt &expr)
{
  auto &side_effect_expr_statement_expression = to_side_effect_expr(expr);
  PRECONDITION(
    side_effect_expr_statement_expression.get_statement() ==
    ID_statement_expression);
  return static_cast<side_effect_expr_statement_expressiont &>(
    side_effect_expr_statement_expression);
}

inline const side_effect_expr_statement_expressiont &
to_side_effect_expr_statement_expression(const exprt &expr)
{
  const auto &side_effect_expr_statement_expression = to_side_effect_expr(expr);
  PRECONDITION(
    side_effect_expr_statement_expression.get_statement() ==
    ID_statement_expression);
  return static_cast<const side_effect_expr_statement_expressiont &>(
    side_effect_expr_statement_expression);
}

/// A \ref side_effect_exprt representation of a function call side effect.
class side_effect_expr_function_callt:public side_effect_exprt
{
public:
  DEPRECATED(SINCE(
    2018,
    8,
    9,
    "use side_effect_expr_function_callt("
    "function, arguments, type, loc) instead"))
  side_effect_expr_function_callt(
    const exprt &_function,
    const exprt::operandst &_arguments,
    const typet &_type)
    : side_effect_exprt(ID_function_call, _type)
  {
    operands().resize(2);
    op1().id(ID_arguments);
    function() = _function;
    arguments() = _arguments;
  }

  side_effect_expr_function_callt(
    exprt _function,
    exprt::operandst _arguments,
    typet _type,
    source_locationt loc)
    : side_effect_exprt(
        ID_function_call,
        {std::move(_function),
         multi_ary_exprt{ID_arguments, std::move(_arguments), typet{}}},
        std::move(_type),
        std::move(loc))
  {
  }

  exprt &function()
  {
    return op0();
  }

  const exprt &function() const
  {
    return op0();
  }

  exprt::operandst &arguments()
  {
    return op1().operands();
  }

  const exprt::operandst &arguments() const
  {
    return op1().operands();
  }
};

template<>
inline bool can_cast_expr<side_effect_expr_function_callt>(const exprt &base)
{
  return detail::can_cast_side_effect_expr_impl(base, ID_function_call);
}

// to_side_effect_expr_function_call only checks the id, so no validate_expr is
// provided for side_effect_expr_function_callt

inline side_effect_expr_function_callt
  &to_side_effect_expr_function_call(exprt &expr)
{
  PRECONDITION(expr.id() == ID_side_effect);
  PRECONDITION(expr.get(ID_statement) == ID_function_call);
  return static_cast<side_effect_expr_function_callt &>(expr);
}

inline const side_effect_expr_function_callt
  &to_side_effect_expr_function_call(const exprt &expr)
{
  PRECONDITION(expr.id() == ID_side_effect);
  PRECONDITION(expr.get(ID_statement) == ID_function_call);
  return static_cast<const side_effect_expr_function_callt &>(expr);
}

/// A \ref side_effect_exprt representation of a side effect that throws an
/// exception.
class side_effect_expr_throwt:public side_effect_exprt
{
public:
  side_effect_expr_throwt(
    irept exception_list,
    typet type,
    source_locationt loc)
    : side_effect_exprt(ID_throw, std::move(type), std::move(loc))
  {
    set(ID_exception_list, std::move(exception_list));
  }
};

template<>
inline bool can_cast_expr<side_effect_expr_throwt>(const exprt &base)
{
  return detail::can_cast_side_effect_expr_impl(base, ID_throw);
}

// to_side_effect_expr_throw only checks the id, so no validate_expr is
// provided for side_effect_expr_throwt

inline side_effect_expr_throwt &to_side_effect_expr_throw(exprt &expr)
{
  PRECONDITION(expr.id() == ID_side_effect);
  PRECONDITION(expr.get(ID_statement) == ID_throw);
  return static_cast<side_effect_expr_throwt &>(expr);
}

inline const side_effect_expr_throwt &to_side_effect_expr_throw(
  const exprt &expr)
{
  PRECONDITION(expr.id() == ID_side_effect);
  PRECONDITION(expr.get(ID_statement) == ID_throw);
  return static_cast<const side_effect_expr_throwt &>(expr);
}

/// Pushes an exception handler, of the form:
/// exception_tag1 -> label1
/// exception_tag2 -> label2
/// ...
/// When used in a GOTO program statement, the corresponding
/// opcode must be CATCH, and the statement's `targets` must
/// be in one-to-one correspondence with the exception tags.
/// The labels may be unspecified for the case where
/// there is no corresponding source-language label, in which
/// case the GOTO statement targets must be set at the same
/// time.
class code_push_catcht:public codet
{
public:
  code_push_catcht():codet(ID_push_catch)
  {
    set(ID_exception_list, irept(ID_exception_list));
  }

  class exception_list_entryt:public irept
  {
  public:
    exception_list_entryt()
    {
    }

    explicit exception_list_entryt(const irep_idt &tag)
    {
      set(ID_tag, tag);
    }

    exception_list_entryt(const irep_idt &tag, const irep_idt &label)
    {
      set(ID_tag, tag);
      set(ID_label, label);
    }

    void set_tag(const irep_idt &tag)
    {
      set(ID_tag, tag);
    }

    const irep_idt &get_tag() const {
      return get(ID_tag);
    }

    void set_label(const irep_idt &label)
    {
      set(ID_label, label);
    }

    const irep_idt &get_label() const {
      return get(ID_label);
    }
  };

  typedef std::vector<exception_list_entryt> exception_listt;

  code_push_catcht(
    const irep_idt &tag,
    const irep_idt &label):
    codet(ID_push_catch)
  {
    set(ID_exception_list, irept(ID_exception_list));
    exception_list().push_back(exception_list_entryt(tag, label));
  }

  exception_listt &exception_list() {
    return (exception_listt &)find(ID_exception_list).get_sub();
  }

  const exception_listt &exception_list() const {
    return (const exception_listt &)find(ID_exception_list).get_sub();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_push_catcht>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_push_catch);
}

// to_code_push_catch only checks the code statement, so no validate_expr is
// provided for code_push_catcht

static inline code_push_catcht &to_code_push_catch(codet &code)
{
  PRECONDITION(code.get_statement() == ID_push_catch);
  return static_cast<code_push_catcht &>(code);
}

static inline const code_push_catcht &to_code_push_catch(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_push_catch);
  return static_cast<const code_push_catcht &>(code);
}

/// Pops an exception handler from the stack of active handlers
/// (i.e. whichever handler was most recently pushed by a
/// `code_push_catcht`).
class code_pop_catcht:public codet
{
public:
  code_pop_catcht():codet(ID_pop_catch)
  {
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_pop_catcht>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_pop_catch);
}

// to_code_pop_catch only checks the code statement, so no validate_expr is
// provided for code_pop_catcht

static inline code_pop_catcht &to_code_pop_catch(codet &code)
{
  PRECONDITION(code.get_statement() == ID_pop_catch);
  return static_cast<code_pop_catcht &>(code);
}

static inline const code_pop_catcht &to_code_pop_catch(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_pop_catch);
  return static_cast<const code_pop_catcht &>(code);
}

/// A statement that catches an exception, assigning the exception
/// in flight to an expression (e.g. Java catch(Exception e) might be expressed
/// landingpadt(symbol_expr("e", ...)))
class code_landingpadt:public codet
{
public:
  code_landingpadt():codet(ID_exception_landingpad)
  {
    operands().resize(1);
  }

  explicit code_landingpadt(exprt catch_expr)
    : codet(ID_exception_landingpad, {std::move(catch_expr)})
  {
  }

  const exprt &catch_expr() const
  {
    return op0();
  }

  exprt &catch_expr()
  {
    return op0();
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_landingpadt>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_exception_landingpad);
}

// to_code_landingpad only checks the code statement, so no validate_expr is
// provided for code_landingpadt

static inline code_landingpadt &to_code_landingpad(codet &code)
{
  PRECONDITION(code.get_statement() == ID_exception_landingpad);
  return static_cast<code_landingpadt &>(code);
}

static inline const code_landingpadt &to_code_landingpad(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_exception_landingpad);
  return static_cast<const code_landingpadt &>(code);
}

/// \ref codet representation of a try/catch block.
class code_try_catcht:public codet
{
public:
  DEPRECATED(SINCE(2018, 12, 2, "use code_try_catcht(try_code) instead"))
  code_try_catcht():codet(ID_try_catch)
  {
    operands().resize(1);
  }

  /// A statement representing try \p _try_code catch ...
  explicit code_try_catcht(codet _try_code)
    : codet(ID_try_catch, {std::move(_try_code)})
  {
  }

  codet &try_code()
  {
    return code_op0();
  }

  const codet &try_code() const
  {
    return code_op0();
  }

  code_declt &get_catch_decl(unsigned i)
  {
    PRECONDITION((2 * i + 2) < operands().size());
    return to_code_decl(to_code(operands()[2*i+1]));
  }

  const code_declt &get_catch_decl(unsigned i) const
  {
    PRECONDITION((2 * i + 2) < operands().size());
    return to_code_decl(to_code(operands()[2*i+1]));
  }

  codet &get_catch_code(unsigned i)
  {
    PRECONDITION((2 * i + 2) < operands().size());
    return to_code(operands()[2*i+2]);
  }

  const codet &get_catch_code(unsigned i) const
  {
    PRECONDITION((2 * i + 2) < operands().size());
    return to_code(operands()[2*i+2]);
  }

  void add_catch(const code_declt &to_catch, const codet &code_catch)
  {
    add_to_operands(to_catch, code_catch);
  }

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

template <>
inline bool can_cast_code<code_try_catcht>(const codet &base)
{
  return detail::can_cast_code_impl(base, ID_try_catch);
}

inline void validate_expr(const code_try_catcht &x)
{
  validate_operands(x, 3, "try-catch must have three or more operands", true);
}

inline const code_try_catcht &to_code_try_catch(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_try_catch);
  const code_try_catcht &ret = static_cast<const code_try_catcht &>(code);
  validate_expr(ret);
  return ret;
}

inline code_try_catcht &to_code_try_catch(codet &code)
{
  PRECONDITION(code.get_statement() == ID_try_catch);
  code_try_catcht &ret = static_cast<code_try_catcht &>(code);
  validate_expr(ret);
  return ret;
}

/// This class is used to interface between a language frontend
/// and goto-convert -- it communicates the identifiers of the parameters
/// of a function or method
class code_function_bodyt : public codet
{
public:
  explicit code_function_bodyt(
    const std::vector<irep_idt> &parameter_identifiers,
    code_blockt _block)
    : codet(ID_function_body, {std::move(_block)})
  {
    set_parameter_identifiers(parameter_identifiers);
  }

  code_blockt &block()
  {
    return to_code_block(to_code(op0()));
  }

  const code_blockt &block() const
  {
    return to_code_block(to_code(op0()));
  }

  std::vector<irep_idt> get_parameter_identifiers() const;
  void set_parameter_identifiers(const std::vector<irep_idt> &);

protected:
  using codet::op0;
  using codet::op1;
  using codet::op2;
  using codet::op3;
};

inline const code_function_bodyt &to_code_function_body(const codet &code)
{
  PRECONDITION(code.get_statement() == ID_function_body);
  DATA_INVARIANT(
    code.operands().size() == 1, "code_function_body must have one operand");
  return static_cast<const code_function_bodyt &>(code);
}

inline code_function_bodyt &to_code_function_body(codet &code)
{
  PRECONDITION(code.get_statement() == ID_function_body);
  DATA_INVARIANT(
    code.operands().size() == 1, "code_function_body must have one operand");
  return static_cast<code_function_bodyt &>(code);
}

#endif // CPROVER_UTIL_STD_CODE_H
