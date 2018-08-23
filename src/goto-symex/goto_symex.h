/*******************************************************************\

Module: Symbolic Execution

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

/// \file
/// Symbolic Execution

#ifndef CPROVER_GOTO_SYMEX_GOTO_SYMEX_H
#define CPROVER_GOTO_SYMEX_GOTO_SYMEX_H

#include <util/options.h>
#include <util/message.h>

#include <goto-programs/abstract_goto_model.h>

#include "goto_symex_state.h"
#include "path_storage.h"
#include "symex_target_equation.h"

class byte_extract_exprt;
class function_application_exprt;
class typet;
class code_typet;
class symbol_tablet;
class code_assignt;
class code_function_callt;
class exprt;
class goto_symex_statet;
class guardt;
class if_exprt;
class index_exprt;
class symbol_exprt;
class member_exprt;
class namespacet;
class side_effect_exprt;
class symex_assignt;
class typecast_exprt;

/// Configuration of the symbolic execution
struct symex_configt final
{
  unsigned max_depth;
  bool doing_path_exploration;
  bool allow_pointer_unsoundness;
  bool constant_propagation;
  bool self_loops_to_assumptions;
  bool simplify_opt;
  bool unwinding_assertions;
  bool partial_loops;
  mp_integer debug_level;

  /// \brief Should the additional validation checks be run?
  ///
  /// If this flag is set the checks for renaming (both level1 and level2) are
  /// executed in the goto_symex_statet (in the assignment method).
  bool run_validation_checks;

  /// \brief Prints out the path that symex is actively taking during execution,
  /// includes diagnostic information about call stack and guard size.
  bool show_symex_steps;

  /// \brief Construct a symex_configt using options specified in an
  /// \ref optionst
  explicit symex_configt(const optionst &options);
};

/// \brief The main class for the forward symbolic simulator
///
/// Higher-level architectural information on symbolic execution is
/// documented in the \ref symex-overview
/// "Symbolic execution module page".
class goto_symext
{
public:
  typedef goto_symex_statet statet;

  goto_symext(
    message_handlert &mh,
    const symbol_tablet &outer_symbol_table,
    symex_target_equationt &_target,
    const optionst &options,
    path_storaget &path_storage)
    : should_pause_symex(false),
      symex_config(options),
      language_mode(),
      outer_symbol_table(outer_symbol_table),
      ns(outer_symbol_table),
      target(_target),
      atomic_section_counter(0),
      log(mh),
      path_storage(path_storage),
      path_segment_vccs(0),
      _total_vccs(std::numeric_limits<unsigned>::max()),
      _remaining_vccs(std::numeric_limits<unsigned>::max())
  {
  }

  virtual ~goto_symext() = default;

  typedef
    std::function<const goto_functionst::goto_functiont &(const irep_idt &)>
    get_goto_functiont;

  /// Return a function to get/load a goto function from the given goto model
  static get_goto_functiont get_goto_function(abstract_goto_modelt &);

  /// \brief symex entire program starting from entry point
  ///
  /// The state that goto_symext maintains has a large memory footprint.
  /// This method deallocates the state as soon as symbolic execution
  /// has completed, so use it if you don't care about having the state
  /// around afterwards.
  virtual void symex_from_entry_point_of(
    const get_goto_functiont &get_goto_function,
    symbol_tablet &new_symbol_table);

  /// Puts the initial state of the entry point function into the path storage
  virtual void initialize_path_storage_from_entry_point_of(
    const get_goto_functiont &get_goto_function,
    symbol_tablet &new_symbol_table);

  /// Performs symbolic execution using a state and equation that have
  /// already been used to symex part of the program. The state is not
  /// re-initialized; instead, symbolic execution resumes from the program
  /// counter of the saved state.
  virtual void resume_symex_from_saved_state(
    const get_goto_functiont &get_goto_function,
    const statet &saved_state,
    symex_target_equationt *saved_equation,
    symbol_tablet &new_symbol_table);

  //// \brief symex entire program starting from entry point
  ///
  /// This method uses the `state` argument as the symbolic execution
  /// state, which is useful for examining the state after this method
  /// returns. The state that goto_symext maintains has a large memory
  /// footprint, so if keeping the state around is not necessary,
  /// clients should instead call goto_symext::symex_from_entry_point_of().
  virtual void symex_with_state(
    statet &,
    const get_goto_functiont &,
    symbol_tablet &);

  /// \brief Have states been pushed onto the workqueue?
  ///
  /// If this flag is set at the end of a symbolic execution run, it means that
  /// symex has been paused because we encountered a GOTO instruction while
  /// doing path exploration, and thus pushed the successor states of the GOTO
  /// onto path_storage. The symbolic execution caller should now choose which
  /// successor state to continue executing, and resume symex from that state.
  bool should_pause_symex;

protected:
  const symex_configt symex_config;

  /// Initialize the symbolic execution and the given state with
  /// the beginning of the entry point function.
  /// \param get_goto_function: producer for GOTO functions
  /// \return Initialized symex state.
  std::unique_ptr<statet>
  initialize_entry_point_state(const get_goto_functiont &get_goto_function);

  /// Invokes symex_step and verifies whether additional threads can be
  /// executed.
  /// \param state: Current GOTO symex step.
  /// \param get_goto_function: function that retrieves function bodies
  void symex_threaded_step(
    statet &state,
    const get_goto_functiont &get_goto_function);

  /// \brief Called for each step in the symbolic execution
  /// This calls \ref print_symex_step to print symex's current instruction if
  /// required, then \ref execute_next_instruction to execute the actual
  /// instruction body.
  /// \param get_goto_function: The delegate to retrieve function bodies (see
  ///   \ref get_goto_functiont)
  /// \param state: Symbolic execution state for current instruction
  virtual void
  symex_step(const get_goto_functiont &get_goto_function, statet &state);

  /// \brief Executes the instruction `state.source.pc`
  /// Case-switches over the type of the instruction being executed and calls
  /// another function appropriate to the instruction type, for example
  /// \ref symex_function_call if the current instruction is a function call,
  /// \ref symex_goto if the current instruction is a goto, etc.
  /// \param get_goto_function: The delegate to retrieve function bodies (see
  ///   \ref get_goto_functiont)
  /// \param state: Symbolic execution state for current instruction
  void execute_next_instruction(
    const get_goto_functiont &get_goto_function,
    statet &state);

  /// Kills any variables in \ref instruction_local_symbols (these are currently
  /// always let-bound variables defined in the course of executing the current
  /// instruction), then clears \ref instruction_local_symbols.
  void kill_instruction_local_symbols(statet &state);

  /// Prints the route of symex as it walks through the code. Used for
  /// debugging.
  void print_symex_step(statet &state);

  messaget::mstreamt &
  print_callstack_entry(const symex_targett::sourcet &target);

public:

  /// language_mode: ID_java, ID_C or another language identifier
  /// if we know the source language in use, irep_idt() otherwise.
  irep_idt language_mode;

protected:

  /// The symbol table associated with the goto-program that we're
  /// executing. This symbol table will not additionally contain objects
  /// that are dynamically created as part of symbolic execution; the
  /// names of those object are stored in the symbol table passed as the
  /// `new_symbol_table` argument to the `symex_*` methods.
  const symbol_tablet &outer_symbol_table;

  /// Initialized just before symbolic execution begins, to point to
  /// both `outer_symbol_table` and the symbol table owned by the
  /// `goto_symex_statet` object used during symbolic execution. That
  /// symbol table must be owned by goto_symex_statet rather than passed
  /// in, in case the state is saved and resumed. This namespacet is
  /// used during symbolic execution to look up names from the original
  /// goto-program, and the names of dynamically-created objects.
  namespacet ns;
  symex_target_equationt &target;
  unsigned atomic_section_counter;

  /// Variables that should be killed at the end of the current symex_step
  /// invocation. Currently this is used for let-bound variables executed during
  /// symex, whose lifetime is at most one instruction long.
  std::vector<symbol_exprt> instruction_local_symbols;

  /// The messaget to write log messages to
  mutable messaget log;

  friend class symex_dereference_statet;

  /// Clean up an expression
  /// \remarks
  /// this does the following:
  ///   a) rename non-det choices
  ///   b) remove pointer dereferencing
  ///   c) clean up byte_extract on the lhs of an assignment
  /// \param expr: The expression to clean up
  /// \param state
  /// \param write
  exprt clean_expr(exprt expr, statet &state, bool write);

  void trigger_auto_object(const exprt &, statet &);
  void initialize_auto_object(const exprt &, statet &);

  /// Given an expression, find the root object and the offset into it.
  ///
  /// The extra complication to be considered here is that the expression may
  /// have any number of ternary expressions mixed with type casts.
  void process_array_expr(statet &, exprt &);
  exprt make_auto_object(const typet &, statet &);
  virtual void dereference(exprt &, statet &);

  void dereference_rec(exprt &, statet &);
  exprt address_arithmetic(
    const exprt &,
    statet &,
    bool keep_array);

  /// Symbolically execute a GOTO instruction
  /// \param state: Symbolic execution state for current instruction
  virtual void symex_goto(statet &state);
  /// Symbolically execute a START_THREAD instruction
  /// \param state: Symbolic execution state for current instruction
  virtual void symex_start_thread(statet &state);
  /// Symbolically execute an ATOMIC_BEGIN instruction
  /// \param state: Symbolic execution state for current instruction
  virtual void symex_atomic_begin(statet &state);
  /// Symbolically execute an ATOMIC_END instruction
  /// \param state: Symbolic execution state for current instruction
  virtual void symex_atomic_end(statet &state);
  /// Symbolically execute a DECL instruction
  /// \param state: Symbolic execution state for current instruction
  virtual void symex_decl(statet &state);
  /// Symbolically execute a DECL instruction for the given symbol or simulate
  /// such an execution for a synthetic symbol
  /// \param state: Symbolic execution state for current instruction
  /// \param expr: The symbol being declared
  virtual void symex_decl(statet &state, const symbol_exprt &expr);
  /// Symbolically execute a DEAD instruction
  /// \param state: Symbolic execution state for current instruction
  virtual void symex_dead(statet &state);
  /// Kill a symbol, as if it had been the subject of a DEAD instruction
  /// \param state: Symbolic execution state
  /// \param symbol_expr: Symbol to kill
  void symex_dead(statet &state, const symbol_exprt &symbol_expr);
  /// Symbolically execute an OTHER instruction
  /// \param state: Symbolic execution state for current instruction
  virtual void symex_other(statet &state);

  void symex_assert(const goto_programt::instructiont &, statet &);

  /// Propagate constants and points-to information implied by a GOTO condition.
  /// See \ref goto_statet::apply_condition for aspects of this which are common
  /// to GOTO and ASSUME instructions.
  /// \param current_state: state prior to the GOTO instruction
  /// \param jump_taken_state: state following taking the GOTO
  /// \param jump_not_taken_state: fall-through state
  /// \param original_guard: the original GOTO condition
  /// \param new_guard: GOTO condition, L2 renamed and simplified
  /// \param ns: global namespace
  void apply_goto_condition(
    goto_symex_statet &current_state,
    goto_statet &jump_taken_state,
    goto_statet &jump_not_taken_state,
    const exprt &original_guard,
    const exprt &new_guard,
    const namespacet &ns);

  /// Try to filter value sets based on whether possible values of a
  /// pointer-typed symbol make the condition true or false. We only do this
  /// when there is only one pointer-typed symbol in \p condition.
  /// \param state: The current state
  /// \param condition: The condition which is being evaluated, which it expects
  ///   will not have been cleaned or renamed. In practice, it's fine if it has
  ///   been cleaned and renamed up to level L1.
  /// \param original_value_set: The value set we will read from
  /// \param jump_taken_value_set: The value set that will be used when the
  ///   condition is true, so we remove any elements which we can tell will
  ///   make the condition false, or nullptr if this shouldn't be done
  /// \param jump_not_taken_value_set: The value set that will be used when the
  ///   condition is false, so we remove any elements which we can tell will
  ///   make the condition true, or nullptr if this shouldn't be done
  /// \param ns: A namespace
  void try_filter_value_sets(
    goto_symex_statet &state,
    exprt condition,
    const value_sett &original_value_set,
    value_sett *jump_taken_value_set,
    value_sett *jump_not_taken_value_set,
    const namespacet &ns);

  virtual void vcc(
    const exprt &,
    const std::string &msg,
    statet &);

  virtual void symex_assume(statet &, const exprt &cond);

  // gotos
  void merge_gotos(statet &);

  virtual void merge_goto(const goto_statet &goto_state, statet &);

  void merge_value_sets(const goto_statet &goto_state, statet &dest);

  void phi_function(const goto_statet &goto_state, statet &);

  // determine whether to unwind a loop -- true indicates abort,
  // with false we continue.
  virtual bool should_stop_unwind(
    const symex_targett::sourcet &source,
    const goto_symex_statet::call_stackt &context,
    unsigned unwind);

  virtual void loop_bound_exceeded(statet &, const exprt &guard);

  virtual void no_body(const irep_idt &)
  {
  }

  /// Symbolic execution of a function call.
  /// Only functions that are symbols are supported, see
  /// \ref goto_symext::symex_function_call_symbol
  virtual void symex_function_call(
    const get_goto_functiont &,
    statet &,
    const code_function_callt &);

  virtual void symex_end_of_function(statet &);

  /// Symbolic execution of a call to a function call.
  /// For functions \c CBMC_trace and functions starting with \c __CPROVER_fkt
  /// see \ref goto_symext::symex_trace and
  /// \ref goto_symext::symex_fkt
  /// For non-special functions see
  /// \ref goto_symext::symex_function_call_code
  virtual void symex_function_call_symbol(
    const get_goto_functiont &,
    statet &,
    const code_function_callt &);

  /// Symbolic execution of a function call by inlining.
  /// Records the call in \p target by appending a function call step and:
  ///   - if the body is available create a new frame, assigns the parameters,
  ///    and proceed to executing the code of the function.
  ///   - otherwise assign a nondetministic value to the left-hand-side of the
  ///     call when there is one
  virtual void symex_function_call_code(
    const get_goto_functiont &,
    statet &,
    const code_function_callt &);

  virtual bool get_unwind_recursion(
    const irep_idt &identifier,
    unsigned thread_nr,
    unsigned unwind);

  /// Iterates over \p arguments and assigns them to the parameters, which are
  /// symbols whose name and type are deduced from the type of \p goto_function.
  /// \param function_identifier: name of the function
  /// \param goto_function: function whose parameters we want to assign
  /// \param [out] state: state of the goto program
  /// \param arguments: arguments that are passed to the function
  void parameter_assignments(
    const irep_idt &function_identifier,
    const goto_functionst::goto_functiont &,
    statet &,
    const exprt::operandst &arguments);

  // exceptions
  void symex_throw(statet &);
  void symex_catch(statet &);

  virtual void do_simplify(exprt &);

  void symex_assign(statet &, const code_assignt &);

  /// Attempt to constant propagate side effects of the assignment (if any)
  ///
  /// \param state: goto symex state
  /// \param symex_assign: object handling symbol assignments
  /// \param lhs: lhs of the assignment
  /// \param rhs: rhs of the assignment
  /// \return true if the operation could be evaluated to a constant string,
  ///   false otherwise
  bool constant_propagate_assignment_with_side_effects(
    statet &state,
    symex_assignt &symex_assign,
    const exprt &lhs,
    const exprt &rhs);

  /// Create an empty string constant
  ///
  /// \param state: goto symex state
  /// \param symex_assign: object handling symbol assignments
  /// \param f_l1: application of function ID_cprover_string_empty_string_func
  ///   with l1 renaming applied
  void constant_propagate_empty_string(
    statet &state,
    symex_assignt &symex_assign,
    const function_application_exprt &f_l1);

  /// Attempt to constant propagate string concatenation
  ///
  /// \param state: goto symex state
  /// \param symex_assign: object handling symbol assignments
  /// \param f_l1: application of function ID_cprover_string_concat_func with
  ///   l1 renaming applied
  /// \return true if the operation could be evaluated to a constant string,
  ///   false otherwise
  bool constant_propagate_string_concat(
    statet &state,
    symex_assignt &symex_assign,
    const function_application_exprt &f_l1);

  /// Attempt to constant propagate getting a substring of a string
  ///
  /// \param state: goto symex state
  /// \param symex_assign: object handling symbol assignments
  /// \param f_l1: application of function ID_cprover_string_substring_func with
  ///   l1 renaming applied
  /// \return true if the operation could be evaluated to a constant string,
  ///   false otherwise
  bool constant_propagate_string_substring(
    statet &state,
    symex_assignt &symex_assign,
    const function_application_exprt &f_l1);

  /// Assign constant string length and string data given by a char array to
  /// given ssa variables
  ///
  /// A new symbol is created (if not yet existing) in the symbol table to hold
  /// the string data given by `new_char_array`. The name of the symbol is
  /// derived from the contents of `new_char_array` (e.g., if the array contains
  /// "abc", the symbol will be named "abc_constant_char_array"). Then, the
  /// expression `&sym[0]` is assigned to `char_array` (assuming `sym` denotes
  /// the symbol holding the string data given by `new_char_array`.
  ///
  /// \param state: goto symex state
  /// \param symex_assign: object handling symbol assignments
  /// \param length: ssa variable to assign the constant string length to
  /// \param new_length: value to assign to `length`
  /// \param char_array: ssa variable to assign the constant string data to
  /// \param new_char_array: constant char array which gives the string data to
  ///   assign to `char_array`
  void assign_string_constant(
    statet &state,
    symex_assignt &symex_assign,
    const ssa_exprt &length,
    const constant_exprt &new_length,
    const ssa_exprt &char_array,
    const array_exprt &new_char_array);

  /// Installs a new symbol in the symbol table to represent the given
  /// character array, and assigns the character array to the symbol
  ///
  /// \param state: goto symex state
  /// \param symex_assign: object handling symbol assignments
  /// \param aux_symbol_name: name of the symbol to create
  /// \param char_array: ssa variable to which to assign a pointer to the symbol
  /// \param new_char_array: new character array to assign to the symbol
  const symbolt &get_new_string_data_symbol(
    statet &state,
    symex_assignt &symex_assign,
    const std::string &aux_symbol_name,
    const ssa_exprt &char_array,
    const array_exprt &new_char_array);

  /// Generate array to pointer association primitive
  ///
  /// Executes an assignment `return_value = f(new_char_array, string_data)`,
  /// with `new_char_array` being the character array to associate with pointer
  /// `string_data`
  ///
  /// \param state: goto symex state
  /// \param symex_assign: object handling symbol assignments
  /// \param new_char_array: character array to associate with pointer
  /// \param string_data: pointer to associate with character array
  void associate_array_to_pointer(
    statet &state,
    symex_assignt &symex_assign,
    const array_exprt &new_char_array,
    const address_of_exprt &string_data);

  optionalt<std::reference_wrapper<const array_exprt>>
  try_evaluate_constant_string(const statet &state, const exprt &content);

  // clang-format off
  static optionalt<std::reference_wrapper<const constant_exprt>>
  try_evaluate_constant(
    const statet &state,
    const exprt &expr);
  // clang-format on

  // havocs the given object
  void havoc_rec(statet &, const guardt &, const exprt &);

  typedef symex_targett::assignment_typet assignment_typet;

  /// Execute any let expressions in \p expr using
  /// \ref symex_assignt::assign_symbol.
  /// The assignments will be made in bottom-up topological but otherwise
  /// arbitrary order (i.e. in `(let x = let y = 0 in x + y) + (let z = 0 in z)
  /// we will define `y` before `x`, but `z` and `x` could come in either order)
  void lift_lets(statet &, exprt &);

  /// Execute a single let expression, which should not have any nested let
  /// expressions (use \ref lift_lets instead if there might be).
  /// Records the newly-defined variable in \ref instruction_local_symbols,
  /// meaning it will be killed when \ref symex_step concludes.
  void lift_let(statet &state, const let_exprt &let_expr);

  virtual void
  symex_va_start(statet &, const exprt &lhs, const side_effect_exprt &);

  /// Symbolically execute an assignment instruction that has an `allocate` on
  /// the right hand side
  /// \param state: Symbolic execution state for current instruction
  /// \param lhs: The expression to assign to
  /// \param code: The `allocate` expression
  virtual void symex_allocate(
    statet &, const exprt &lhs, const side_effect_exprt &);
  virtual void symex_cpp_delete(statet &, const codet &);
  virtual void symex_cpp_new(
    statet &, const exprt &lhs, const side_effect_exprt &);
  virtual void symex_fkt(statet &, const code_function_callt &);
  virtual void symex_trace(statet &, const code_function_callt &);
  virtual void symex_printf(statet &, const irept &);
  virtual void symex_input(statet &, const codet &);
  virtual void symex_output(statet &, const codet &);

  static unsigned dynamic_counter;

  void rewrite_quantifiers(exprt &, statet &);

  path_storaget &path_storage;

public:
  /// \brief Number of VCCs generated during the run of this goto_symext object
  ///
  /// This member is always initialized to `0` upon construction of this object.
  /// It therefore differs from goto_symex_statet::total_vccs, which persists
  /// across the creation of several goto_symext objects. When CBMC is run in
  /// path-exploration mode, the meaning of this member is "the number of VCCs
  /// generated between the last branch point and the current instruction,"
  /// while goto_symex_statet::total_vccs records the total number of VCCs
  /// generated along the entire path from the beginning of the program.
  std::size_t path_segment_vccs;

protected:
  /// @{\name Statistics
  ///
  /// The actual number of total and remaining VCCs should be assigned to
  /// the relevant members of goto_symex_statet. The members below are used to
  /// cache the values from goto_symex_statet after symex has ended, so that
  /// \ref bmct can read those values even after the state has been deallocated.

  unsigned _total_vccs, _remaining_vccs;
  ///@}

public:
  unsigned get_total_vccs()
  {
    INVARIANT(
      _total_vccs != std::numeric_limits<unsigned>::max(),
      "symex_threaded_step should have been executed at least once before "
      "attempting to read total_vccs");
    return _total_vccs;
  }

  unsigned get_remaining_vccs()
  {
    INVARIANT(
      _remaining_vccs != std::numeric_limits<unsigned>::max(),
      "symex_threaded_step should have been executed at least once before "
      "attempting to read remaining_vccs");
    return _remaining_vccs;
  }

  void validate(const validation_modet vm) const
  {
    target.validate(ns, vm);
  }
};

/// Transition to the next instruction, which increments the internal program
/// counter and initializes the loop counter when it detects a loop (or
/// recursion) being entered. 'Next instruction' in this situation refers
/// to the next one in program order, so it ignores things like unconditional
/// GOTOs, and only goes until the end of the current function.
void symex_transition(goto_symext::statet &state);

void symex_transition(
  goto_symext::statet &,
  goto_programt::const_targett to,
  bool is_backwards_goto);

/// Try to evaluate pointer comparisons where they can be trivially determined
/// using the value-set. This is optional as all it does is allow symex to
/// resolve some comparisons itself and therefore create a simpler formula for
/// the SAT solver.
/// \param [in,out] condition: An L2-renamed expression with boolean type
/// \param value_set: The value-set for determining what pointer-typed symbols
///   might possibly point to
/// \param language_mode: The language mode
/// \param ns: A namespace
/// \return The possibly modified condition
renamedt<exprt, L2> try_evaluate_pointer_comparisons(
  renamedt<exprt, L2> condition,
  const value_sett &value_set,
  const irep_idt &language_mode,
  const namespacet &ns);

#endif // CPROVER_GOTO_SYMEX_GOTO_SYMEX_H
