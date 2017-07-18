#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import sys
import os
import subprocess

from subprocess import call

from flecsit.services.analysis_driver.cmakelist import *

def dir_exists(path):
    print path
    if not os.path.exists(path):
        os.makedirs(path) 
        
def sym_exists(source,link_name):
    if not os.path.exists(link_name):
        os.symlink(source,link_name)
        
def get_lib_name(lib):
    left = 0
    if(lib.rfind('/') != -1):
        left = lib.rfind('/')
    elif(lib.rfind('-l') != -1):
        left = lib.find('-l')+1
        
    right = len(lib)
    if(lib.find('.so') != -1):
        right = lib.find('.so')
        
    print len(lib), left, right
        
    return (lib[(left+1):(right)]).replace('lib','')
    
def get_lib_path(lib):
    if(lib.find('/') != -1):
        return lib[0:lib.rfind('/')]
    return ''
        
def execute(verbose, project_name, build):

    """
    """
    
    #----------------------------------------------------------------------#
    # Process command-line arguments
    #----------------------------------------------------------------------#

    project_header = project_name + '.cc'
    
    # We first need to set up the directory structure for the 
    project_dir = os.getcwd() + '/' + project_name
    build_dir = project_dir + '/build'
    
    dir_exists(project_dir)
    dir_exists(build_dir)
    
    flecsi_install = os.path.realpath(__file__).partition("lib")[0].rstrip("/")
    
    flecsi_runtime = flecsi_install + '/share/flecsi/runtime'
    
    # Create symbolic links to the runtime source files
    sym_exists(flecsi_runtime+'/runtime_driver.cc', project_dir + '/runtime_driver.cc')
    sym_exists(flecsi_runtime+'/runtime_main.cc', project_dir + '/runtime_main.cc')
    
    # Create a symbolic link to the header file
    sym_exists(os.getcwd() +'/' + project_header, project_dir + '/' + project_header)
    
    # We need to put the library and include paths into the proper format for cmake
    mangled_list = build['libraries'].split(" ")
    cmake_lib_dirs = ""
    
    for tmp in mangled_list:
        if(tmp.find('-L')):
            cmake_lib_dirs += tmp.strip('-L') + ' '
    
    for tmp in mangled_list:
        res = get_lib_name(tmp)
        print tmp, res, get_lib_path(tmp)
    
    cmake_include_dirs = build['includes'].replace('-I', '')
    
    cmake_include_dirs += " " +flecsi_install + '/include'
    
    cmake_defines = build['defines']
    
    source_output = cmake_source_template.substitute(
        CMAKE_VERSION="VERSION 2.8",
        PROJECT_NAME=project_name,
        CMAKE_INCLUDE_DIRS=cmake_include_dirs,
        CMAKE_DEFINES=cmake_defines)
        
    print build['libraries']
    
    fd = open(project_dir+'/CMakeLists.txt','w')
    fd.write(source_output[1:-1])
    fd.close()    

    os.chdir(build_dir)
    
    command1 = 'cmake ..'
    
    subprocess.call(command1.split())
    
    
    # get the location of the llvm install
    llvm_includes = subprocess.check_output(('llvm-config --includedir').split()).rstrip("\r\n") + '/c++/v1'
# execute
    
    command = 'CC=gcc blend -extra-arg-before=\"-I' + \
               llvm_includes + '\" ' + \
               '-extra-arg-before=\"-I' + flecsi_install + '/include\" ' + \
               '-extra-arg-before=\"-w\" ' +\
               ' ../' + project_name + '.cc'
    
    print command
    
    subprocess.call(command,shell=True)

#------------------------------------------------------------------------------#
# Formatting options for vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
