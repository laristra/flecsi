.. |br| raw:: html

   <br />

Runtime Model
*************

With the following CMake options enabled:

.. code-block:: console

  $ cmake .. -DENABLE_FLOG=ON -DENABLE_GRAPHVIZ=ON

an executable compiled with FleCSI will have several command-line options available. For example, running the *color* unit test from its location in *test/execution* with the *-h* flag, will produce the following output:

.. code-block:: console

  $ ./color -h
    color:
    -h [ --help ]            Print this message and exit.
    -t [ --tags ] [=arg(=0)] Enable the specified output tags, e.g.,
                             --tags=tag1,tag2. Passing --tags by itself will
                             print the available tags.
    --control-model          Output the current control model and exit.

The *--tags* option allows users to control logging output, e.g., by
turning on or off certain *guarded* outputs. This is a feature provided
by the FleCSI :ref:`flog`.  The *--control-model* option instructs the
executable to output a *.dot* file of the control-flow graph of the
control model. The FleCSI :ref:`control-model` allows users to define
the structure of execution of a program. Additional options may be added
in the future, and will be documented in this guide.  

----

Custom Handlers
+++++++++++++++

Because the FleCSI programming system requires, in most cases, that the
user relinquish direct control of the *main* function, we have provided
a runtime interface that allows fairly fine-grained customization of the
initialization, execution, and finalization of a program. Formally, the
runtime model is part of the `Cinch build system utility
<https://github.com/laristra/cinch>`_ that was developed to support
FleCSI. The runtime interface allows users to register additional
control-point actions, i.e., initialization, and finalization, in
addition to adding new command-line options. This system allows easy
extensibility to accommodate new runtime systems that may be required by
computing architectures in the future.

As a simple example, consider the following code block:

.. code-block:: cpp
  :linenos:
  :emphasize-lines: 26,27,28,30

  using namespace cinch;
  using namespace boost::program_options;

  inline void custom_add_options(options_descriptions & desc) {

    desc.add_options()("flag,f", "Custom command-line option");

  } // custom_add_options

  inline int custom_initialization(int argc, char ** argv,
    variables_map & vm) {

    if(vm.count("flag") {
      std::cout << "Custom flag passed to program!" << std::endl;
    }

    return 0;
  } // custom_initialization

  inline int custom_finalization(int argc, char ** argv,
    exit_mode_t mode) {

    return 0;
  } // custom_finalization

  inline runtime_handler_t custom_handler {
    custom_initialization, custom_finalization, custom_add_options
  };

  cinch_append_runtime_handler(custom_handler);

This code defines three functions:

* **custom_add_options** |br|
  This function provides a mechanism to add additional command-line
  options to the main Boost options descriptor. The interface for the
  options_descriptions type is documented `here
  <https://www.boost.org/doc/libs/1_69_0/doc/html/program_options.html>`_.

* **custom_initialization** |br|
  This function will be invoked during the initialization phase of the
  Cinch runtime *main* function (The full code of runtime.cc is included
  below.) Users can test command-line options or invoke initialization
  of additional low-level runtime systems here. Non-zero returns from
  this function will cause the top-level execution to exit.

* **custom_finalization** |br|
  This function will be invoked during the shutdown phase of the Cinch
  runtime *main* function. The exit mode of the top-level runtime is
  passed into this function through the *mode* argument. Users can adapt
  shutdown of additional low-level runtime systems based on the exit
  status of the top-level execution.

.. note::

  The names of the custom handler functions are arbitrary, and should
  reflect the user's requirements.

After defining add_options, initialization, and finalization functions,
the user can create a handler object (highlighted lines), and register it
with the Cinch runtime system. 

The following code block shows the actual *main* function implementation
of the Cinch runtime. Lines 39, 69, 71, and 81 are highlighted to
identify the call sites of user-registered handlers:

.. code-block:: cpp
  :linenos:
  :emphasize-lines: 39,69,71,81

  /*
      :::::::: ::::::::::: ::::    :::  ::::::::  :::    :::
     :+:    :+:    :+:     :+:+:   :+: :+:    :+: :+:    :+:
     +:+           +:+     :+:+:+  +:+ +:+        +:+    +:+
     +#+           +#+     +#+ +:+ +#+ +#+        +#++:++#++
     +#+           +#+     +#+  +#+#+# +#+        +#+    +#+
     #+#    #+#    #+#     #+#   #+#+# #+#    #+# #+#    #+#
      ######## ########### ###    ####  ########  ###    ###

     Copyright (c) 2016, Los Alamos National Security, LLC
     All rights reserved.
                                                                                */

  #include <cinch-config.h>
  #include <cinch/runtime.h>

  #include <iostream>
  #include <string>

  #if defined(CINCH_ENABLE_BOOST)
    #include <boost/program_options.hpp>
    using namespace boost::program_options;
  #endif

  using namespace cinch;

  int main(int argc, char ** argv) {

    runtime_t & runtime_ = runtime_t::instance();

  #if defined(CINCH_ENABLE_BOOST)
    std::string program(argv[0]);
    options_description desc(program.substr(program.rfind('/')+1).c_str());

    // Add help option
    desc.add_options()("help,h", "Print this message and exit.");

    // Invoke add options functions
    runtime_.add_options(desc);

    variables_map vm;
    parsed_options parsed =
      command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    store(parsed, vm);

    notify(vm);

    // Gather the unregistered options, if there are any, print a help message
    // and die nicely.
    std::vector<std::string> unrecog_options =
      collect_unrecognized(parsed.options, include_positional);
    if(unrecog_options.size()) {
      std::cout << std::endl << "Unrecognized options: ";
      for ( int i=0; i<unrecog_options.size(); ++i ) {
        std::cout << unrecog_options[i] << " ";
      }
      std::cout << std::endl << std::endl << desc << std::endl;
    } // if

    if(vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    } // if
  #endif

    // Invoke registered runtime initializations
    if(
  #if defined(CINCH_ENABLE_BOOST)
      runtime_.initialize_runtimes(argc, argv, vm)
  #else
      runtime_.initialize_runtimes(argc, argv)
  #endif
    ) {
      std::exit(1);
    } // if

    // Invoke the primary callback
    int result = runtime_.driver()(argc, argv);

    // Invoke registered runtime finalizations
    if(runtime_.finalize_runtimes(argc, argv, exit_mode_t::success)) {
      std::exit(1);
    } // if

    return result;
  } // main  

This code block shows the implementation of the registration interface:

.. code-block:: cpp
  :linenos:

  /*
      :::::::: ::::::::::: ::::    :::  ::::::::  :::    :::
     :+:    :+:    :+:     :+:+:   :+: :+:    :+: :+:    :+:
     +:+           +:+     :+:+:+  +:+ +:+        +:+    +:+
     +#+           +#+     +#+ +:+ +#+ +#+        +#++:++#++
     +#+           +#+     +#+  +#+#+# +#+        +#+    +#+
     #+#    #+#    #+#     #+#   #+#+# #+#    #+# #+#    #+#
      ######## ########### ###    ####  ########  ###    ###

     Copyright (c) 2016, Los Alamos National Security, LLC
     All rights reserved.
                                                                                */
  #pragma once

  /*! @file */

  #include <cinch-config.h>

  #if defined(CINCH_ENABLE_BOOST)
    #include <boost/program_options.hpp>
    using namespace boost::program_options;
  #endif

  #include <functional>
  #include <string>
  #include <vector>

  namespace cinch {

  enum exit_mode_t : size_t {
    success,
    unrecognized_option,
    help
  }; // enum exit_mode_t

  /*!
    Type to define runtime initialization and finalization handlers.
   */

  struct runtime_handler_t {
  #if defined(CINCH_ENABLE_BOOST)
    std::function<int(int, char **, variables_map &)> initialize;
  #else
    std::function<int(int, char **)> initialize;
  #endif
    std::function<int(int, char **, exit_mode_t)> finalize;
  #if defined(CINCH_ENABLE_BOOST)
    std::function<void(options_description &)> add_options =
      [](options_description &){};
  #endif  
  }; // struct runtime_handler_t

  /*!
    The runtime_t type provides a stateful interface for registering and
    executing user-defined actions at initialization and finalization
    control points.
   */

  struct runtime_t {

    static runtime_t & instance() {
      static runtime_t r;
      return r;
    } // instance

    std::string const & program() const { return program_; }
    std::string & program() { return program_; }

    bool register_driver(std::function<int(int, char **)> const & driver) {
      driver_ = driver;
      return true;
    } // register_driver

    std::function<int(int, char **)> const & driver() const {
      return driver_;
    } // driver

    /*!
      Append the given runtime handler to the vector of handlers. Handlers
      will be executed in the order in which they are appended.
     */

    bool append_runtime_handler(runtime_handler_t const & handler) {
      handlers_.push_back(handler);
      return true;
    } // register_runtime_handler

    /*!
      Access the runtime handler vector.
     */

    std::vector<runtime_handler_t> & runtimes() {
      return handlers_;
    } // runtimes

    /*!
      Invoke runtime options callbacks.
     */

  #if defined(CINCH_ENABLE_BOOST)
    void add_options(options_description & desc) {
      for(auto r: handlers_) {
        r.add_options(desc);
      } // for
    } // add_options
  #endif // CINCH_ENABLE_BOOST

    /*!
      Invoke runtime intiailzation callbacks.
     */

  #if defined(CINCH_ENABLE_BOOST)
    int initialize_runtimes(int argc, char ** argv, variables_map & vm) {
      int result{0};

      for(auto r: handlers_) {
        result |= r.initialize(argc, argv, vm);
      } // for

      return result;
    } // initialize_runtimes
  #else
    int initialize_runtimes(int argc, char ** argv) {
      int result{0};

      for(auto r: handlers_) {
        result |= r.initialize(argc, argv);
      } // for

      return result;
    } // initialize_runtimes
  #endif

    /*!
      Invoke runtime finalization callbacks.
     */

    int finalize_runtimes(int argc, char ** argv, exit_mode_t mode) {
      int result{0};

      for(auto r: handlers_) {
        result |= r.finalize(argc, argv, mode);
      } // for

      return result;
    } // finalize_runtimes

  private:

    runtime_t() {}

    ~runtime_t() {}

    // These are deleted because this type is a singleton, i.e.,
    // we don't want anyone to be able to make copies or references.

    runtime_t(const runtime_t &) = delete;
    runtime_t & operator=(const runtime_t &) = delete;
    runtime_t(runtime_t &&) = delete;
    runtime_t & operator=(runtime_t &&) = delete;

    std::string program_;
    std::function<int(int, char **)> driver_;
    std::vector<runtime_handler_t> handlers_;

  }; // runtime_t

  } // namespace cinch

  /*!
    @def cinch_register_runtime_driver(driver)

    Register the primary runtime driver function.

    @param driver The primary driver with a 'int(int, char **)' signature
                  that should be invoked by the FleCSI runtime.
   */

  #define cinch_register_runtime_driver(driver)                                  \
    /* MACRO IMPLEMENTATION */                                                   \
                                                                                 \
    inline bool cinch_registered_driver_##driver =                               \
      cinch::runtime_t::instance().register_driver(driver)

  /*!
    @def cinch_register_runtime_handler(handler)

    Register a runtime handler with the FleCSI runtime. Runtime handlers
    are invoked at fixed control points in the FleCSI control model for
    add options, initialization, and finalization. The finalization function
    has an additional argument that specifies the exit mode. Adding options
    is only enabled with CINCH_ENABLE_BOOST.

    @param handler A runtime_handler_t that references the appropriate
                   initialize, finalize, and add_options functions.
   */

  #define cinch_append_runtime_handler(handler)                                  \
    /* MACRO DEFINITION */                                                       \
                                                                                 \
    inline bool cinch_append_runtime_handler_##handler =                         \
      cinch::runtime_t::instance().append_runtime_handler(handler)

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
