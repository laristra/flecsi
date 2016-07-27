# FleCSI Coding Style

## Naming

FleCSI follows a C-style naming convention of all lower-case letters
with underscores:

    my_type_t

## Templates

Template type names should end with a double underscore:

    template<typename T> class my_template__ {};

The double underscore denotes an unqualified type, allowing the user to
create fully qualified type names like:

    using my_template_t = my_template__<parameter>;

The double underscore was chosen so that it does not conflict with
aggregate data names, which use a single underscore.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
