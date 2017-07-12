/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef blender_state_h
#define blender_state_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 12, 2017
//----------------------------------------------------------------------------//

namespace blender {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct state_t
{
  //--------------------------------------------------------------------------//
  //! Meyer's singleton instance.
  //!
  //! @return The single instance of this type.
  //--------------------------------------------------------------------------//

  static
  state_t &
  instance()
  {
    static state_t state;
    return state;
  } // instance

private:

  //! Default constructor
  state_t() {}

  //! Destructor
  ~state_t() {}

  // We don't need any of these
  state_t(const state_t &) = delete;
  state_t & operator = (const state_t &) = delete;
  state_t(state_t &&) = delete;
  state_t & operator = (state_t &&) = delete;

  std::unordered_map<size_t, task_info_t> registered_tasks_;
  std::unordered_map<size_t, function_info_t> registered_functions_;
  std::unordered_map<size_t, field_info_t> registered_fields_;
  std::unordered_map<size_t, client_info_t> registered_clients_;

}; // class state_t

} // namespace blender

#endif // blender_state_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
