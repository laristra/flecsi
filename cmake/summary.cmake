include(colors)

macro(summary_header)
  string(APPEND _summary
    "${FLECSI_BoldCyan}"
    "\n"
"
#------------------------------------------------------------------------------#
"
    "# FleCSI Configuration Summary"
"
#------------------------------------------------------------------------------#
"
    "\n"
    "${FLECSI_ColorReset}"
)
endmacro()

macro(summary_info name info allow_split)
  if(NOT ${info} STREQUAL "")
    string(REPLACE " " ";" split ${info})
    list(LENGTH split split_length)
    string(LENGTH ${name} name_length)

    string(APPEND _summary
      "${FLECSI_Plain}"
      "  ${name}:"
      "${FLECSI_Brown} "
    )

    if(${allow_split} AND split_length GREATER 1)
      math(EXPR split_minus "${split_length}-1")
      list(GET split ${split_minus} last)
      list(REMOVE_AT split ${split_minus})

      set(fill " ")
      string(LENGTH ${fill} fill_length)
      while(${fill_length} LESS ${name_length})
        string(APPEND fill " ")
        string(LENGTH ${fill} fill_length)
      endwhile()

      string(APPEND _summary "${FLECSI_Brown}")
      foreach(entry ${split})
        string(APPEND _summary
          "${entry}\n${fill}    "
          )
      endforeach()
      string(APPEND _summary "${last}${FLECSI_ColorReset}\n")
    else()
      string(APPEND _summary
        "${info}"
        "${FLECSI_ColorReset}"
        "\n"
      )
    endif()
  endif()
endmacro()

macro(summary_option name state extra)
  string(APPEND _summary
    "${FLECSI_Plain}"
    "  ${name}:"
    "${FLECSI_ColorReset}"
  )

  if(${state})
    string(APPEND _summary
      "${FLECSI_Green}"
      " ${state}"
      "${FLECSI_ColorReset}"
      "${extra}"
    )
  else()
    string(APPEND _summary
      "${FLECSI_BoldGrey}"
      " ${state}"
      "${FLECSI_ColorReset}"
    )
  endif()
  string(APPEND _summary "\n")
endmacro()
