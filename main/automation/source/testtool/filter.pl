#!/usr/bin/perl
#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

$debug = "";
$ctrue = 1;
$cfalse = 0;
# reads a block
# Parameter:   FileHandle
#              list of Regular Expressions which terminate the block.
#   for '#ifdef' block would then be ("^#else\$", "^#endif\$")

sub read_block {

  local($file) = @_;
  print "reading block '$file' ",scalar(@_),"\n" if $debug;
  while ( <$file> ) {
    chop;
    s/\s*$//;             # remove trailing whitespaces
    s/^\s*//;             # remove leading whitespaces
    print "Input : \"$_\"\n" if $debug;
    s/\/\/.*//;         # Remove line comment
    s/\/\*.*?\*\///g;   # remove comments within one line
    s/\s+/ /g;          # Change all whitespace into a single blank
    s/ *$//;            # Remove whitespace at end
    s/^# /#/;           # Change # <command> to #<command>


    @line = split(/ /,$_,3);

    $_ = $line[0];
    if ( $_ && /^#/ ) {                # Line starts with '#' -> preprocessor command
      print "proccessing line: @line\n" if $debug;
      if (/#define/)
      {
        if ( $line[1] =~ /^$namefilter/ )
        {
          $mykey = $line[1];
          $mykey =~ s/^$namefilter//;
          $count += 1;
          print OUT "$mykey    ", $line[2], "\n";
          print OUT2 "\t{ \"$mykey\", ", $line[2] ," },\n";
        }
      }
    }
  }
  print "Leaving read_block at the end\n" if $debug;
}

# Read a file.
# first parameter ist the filename
sub read_file {

  local ($filename,$file) = @_;
  $file++;                           # String increment
  local $TempFileName = $basename."/".$filename;
  print "reading file $TempFileName as $file\n" if $debug;
  open($file, $TempFileName) || die "error: Could not open file $TempFileName. ";

  &read_block($file);         # read data
  close($file);
  print "done reading $filename\n" if $debug;
}

# main starts here

$basename = ".";
$basename = $ARGV[0] if defined($ARGV[0]);

$filename = "app.hrc";
$filename = $ARGV[1] if defined($ARGV[1]);


$outfilebase = $filename;
$outfilebase =~ s/\.[^.]+$//;           # cut off suffix
$outfilebase = $ARGV[2] if defined($ARGV[2]);


$namefilter = $ARGV[3] if defined($ARGV[3]);


print "Generating $outfilebase:\n";

$count = 0;

open(OUT,">$outfilebase");
open(OUT2,">$outfilebase.hxx");
print OUT2 "\{\n";

&read_file ($filename,"f00");

print OUT2 "\t{ \"\" ,0 }\n\};\n";

close(OUT);
close(OUT2);

if ( $count == 0 )
{
  die "Error: No Entries Found generating \"$outfilebase.hxx\". Testtool will not work!"
}

