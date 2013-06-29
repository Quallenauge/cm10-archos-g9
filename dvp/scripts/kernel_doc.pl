#!/usr/bin/perl -w

# Copyright (C) 2009-2012 Texas Instruments, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# v01   J.Villarreal    - Initial Release
# v02   J.Villarreal    - Changes to support dvp removal of union
# v03   J.Villarreal    - Changes to remove dependency on xls file, extract
#                         all necessary info from the code.
# v04   J.Villarreal    - Changes to leave the type blank if it doesn't
#                         exist in the comments.
# v05   J.Villarreal    - Adding GPU core
# v06   J.Villarreal    - Updating header file parse start for named enums

use Cwd;

$argc = @ARGV;
if ($argc < 1) {
  die "\nusage:   perl kernel_doc.pl <kernel doc output file> <dvp_types.h> <additional header files> \nversion: v06\n\n";
}

my %hash_enums = ();
my %hash_types = ();
my @headers;

my $outFile = shift;
my $summary = shift;
my $errorlog = "error.log";

my $k;
my $j;
my $i;
my $m;
my $type;
my $data;
my $cpu;
my $dsp;
my $imx;
my $core = "\\ref DVP_CORE_";
my $cmma1;
my $cmma2;
my $parseOrder;
my $endOrder;
my $libName;
my $libNameTmp;

for( $k=3; $k<=$argc; $k++ ) {
    push (@headers, shift);
}

open(DOC, ">$outFile") || die("can't open file");

$dir = cwd; # Assume this is running from the DVP root directory.

&parseHeader($summary, "^typedef enum _dvp_knode_e", "^} DVP_KernelNode_e");    #parse dvp_types.h
push(@endOrder, scalar @parseOrder);
push(@libName, "DVP Common Kernels");
foreach $k (@headers) {
    &parseHeader($k, "^enum ", "^};");
    push(@endOrder, scalar @parseOrder);
    push(@libName, "$libNameTmp Library");
}
open(ERLOG, ">$errorlog") || die("can't open file");

&parseKgm("$dir/source/dvp/dvp_kgm_cpu/dvp_kgm_cpu.c");
&parseKgm("$dir/source/dvp/dvp_kgm_dsp/dvp_kgm_dsp.c");
&parseKgm("$dir/source/dvp/dvp_kgm_simcop/dvp_kgm_simcop.c");
&parseKgm("$dir/source/dvp/dvp_kgm_ocl/dvp_kgm_ocl.c");
#&parseSummary($summary);

close(ERLOG);

print DOC "/*! \\page kernel_page\n
\\section kernel_table Kernel Status Tables
The following tables contains a subset of the information found in the \\ref DVP_KernelNode_e reference,
including all the available kernel enumerations, their associated DVP_KernelNode_t::data types, and
what cores they are available to run on.
\\note 'x' means not available.\n
";

# Print Complete Kernel Tables
$i = 0;
$m = 0;
foreach $j (@endOrder) {
    print DOC "\\subsection lib$m $libName[$m]\n
";
    print DOC "<table border=\"1\">\n";
    print DOC "<tr>
<td><b>#</b></td>
<td><b>\\ref DVP_KernelNode_e</b></td>
<td><b>DVP_KernelNode_t::data Type</b></td>
<td><b>\\ref DVP_CORE_CPU</b></td>
<td><b>\\ref DVP_CORE_DSP</b></td>
<td><b>\\ref DVP_CORE_SIMCOP</b></td>
<td><b>\\ref DVP_CORE_GPU</b></td>
</tr>
";
    # Clear the tableSum
    @tableSum = (0, 0, 0, 0);

    for($tbCnt=1 ; $i<$j; $i++) {
        $k = $parseOrder[$i];
        print DOC "<tr>
<td>$tbCnt</td>
<td>\\ref $k</td>
<td>$hash_enums{$k}[0]</td>
<td><center>$hash_enums{$k}[1]</center></td>
<td><center>$hash_enums{$k}[2]</center></td>
<td><center>$hash_enums{$k}[3]</center></td>
<td><center>$hash_enums{$k}[4]</center></td>
</tr>
";
        for($zzz=1 ; $zzz<=4; $zzz++) {
            $tableSum[$zzz-1] += ($hash_enums{$k}[$zzz] ne "x") ? 1 : 0;
        }
        $tbCnt++;
    }

    print DOC "<tr>
<td></td>
<td></td>
<td></td>
<td><center>$tableSum[0]</center></td>
<td><center>$tableSum[1]</center></td>
<td><center>$tableSum[2]</center></td>
<td><center>$tableSum[3]</center></td>
</tr>
";

    print DOC "</table>\n";
    $m++;
}



# Print comments for each enumeration, cross-referencing the required data types and available cores.
foreach $k (keys %hash_enums) {
    $type = ($hash_enums{$k}[0] eq 'x') ? "N/A" : $hash_enums{$k}[0];
    $cpu  = ($hash_enums{$k}[1] eq 'x') ? "" :
            ($hash_enums{$k}[1] eq 'NEON') ? "$core"."CPU (NEON)" : "$core"."CPU";
    $dsp  = ($hash_enums{$k}[2] eq 'x') ? "" : "$core"."$hash_enums{$k}[2]";
    $imx  = ($hash_enums{$k}[3] eq 'x') ? "" : "$core"."$hash_enums{$k}[3]";
    $gpu  = ($hash_enums{$k}[4] eq 'x') ? "" : "$core"."$hash_enums{$k}[4]";
    $cmma1 = ($cpu && $dsp) ? ", " : "";
    $cmma2 = (($cpu || $dsp) && $imx) ? ", " : "";
    $cmma3 = (($cpu || $dsp || $imx) && $gpu) ? ", " : "";
    print DOC "
    \\var DVP_KernelNode_e::$k
    Implemented on following cores: $cpu"."$cmma1"."$dsp"."$cmma2"."$imx"."$cmma3"."$gpu
";
}

# Print comments for each data types, cross-referncing the kernel enumerations that require each data type.
foreach $k (keys %hash_types) {
    local $, = "\\n \n    - \\ref ";
    print DOC "
    \\class $k
    Used by the following kernels:\\n
    - \\ref ";
    print DOC @{$hash_types{$k}};
    print DOC "\n";
}

print DOC "*/";

sub parseHeader {
    my $header = shift;
    my $start = shift;
    my $end = shift;
    my $values;
    my $configStruct;

    open(HEAD, $header) || die("can't open file");

    $active = 0;
    while ($line=<HEAD>) {
        if($line =~ /$start/) {
            $active = 1;
        }
        elsif ($line =~ /$end/) {
            $active = 0;
        }

        if($active)
        {
            if($line =~ /\s+Configuration Structure: (DVP_\w+)/) {
                $configStruct = $1;
            }
            elsif($line =~ /\s+(DVP_KN_\w+),?/ &&
                  $line !~ /\s+DVP_KN_\w+_BASE/ &&
                  $line !~ /\s+DVP_KN_INVALID/ &&
                  $line !~ /\s+DVP_KN_MAXIMUM/) {
                $values = [];   #creates a new anonymous array
                push (@$values, $configStruct);
                push (@$values, "x"); #placeholder for CPU
                push (@$values, "x"); #placeholder for DSP
                push (@$values, "x"); #placeholder for SIMCOP
                push (@$values, "x"); #placeholder for GPU
                $hash_enums{$1} = $values;
                push @{ $hash_types{$configStruct} }, $1;
                push (@parseOrder, $1);
                $configStruct = ""; # Clearing variable for next enum
            }
            elsif($line =~ /DVP_KN_(\w+)_BASE =/) {
                $libNameTmp = $1;
            }
        }
    }
    close(HEAD);
}

sub parseKgm {
    my $kgm = shift;
    my $slot;
    my $core;
    my %case_enums = ();

    if($kgm =~ /cpu/) {
        $slot = 1;
        $core = "CPU";
    }
    elsif($kgm =~ /dsp/) {
        $slot = 2;
        $core = "DSP";
    }
    elsif($kgm =~ /simcop/) {
        $slot = 3;
        $core = "SIMCOP";
    }
    elsif($kgm =~ /ocl/) {
        $slot = 4;
        $core = "GPU";
    }

    open(KGM, $kgm) || die("can't open file");

    $active = 0;
    while ($line=<KGM>) {
        if($line =~ /static DVP_CoreFunction_t/) {
            $active = 1;
        }
        elsif ($line =~ /};/) {
            $active = 0;
        }

        if($active)
        {
            if($line =~ /(DVP_KN_\w+)/ &&
               $line !~ /\/\// &&
               exists $hash_enums{ $1 }) {
                $hash_enums{$1}[$slot] = $core;
            }
        }

        # This portion is used for static code analysis
        if($line =~ /case (DVP_KN_\w+)/ &&
           $line !~ /\@todo/ &&
           exists $hash_enums{ $1 }) {
            $case_enums{$1} = $core;
        }
    }
    close(KGM);

    # This portion is used for static code analysis
    print ERLOG "\n+++++++++++++++++++++++++++++++++++++++++++\n";
    print ERLOG "$core \t Checking for missing cases\n";
    print ERLOG "+++++++++++++++++++++++++++++++++++++++++++\n";
    foreach $k (keys %hash_enums) {
        if( ($hash_enums{$k}[$slot] ne "x") &&
           !(exists $case_enums{ $k }) )
        {
            print ERLOG "$core \t missing case for $k\n";
        }
    }
    print ERLOG "\n+++++++++++++++++++++++++++++++++++++++++++\n";
    print ERLOG "$core \t Checking for missing table entries\n";
    print ERLOG "+++++++++++++++++++++++++++++++++++++++++++\n";
    foreach $k (keys %case_enums) {
        if( !(exists $hash_enums{ $k }) ||
            ($hash_enums{$k}[$slot] eq "x") )
        {
            print ERLOG "$core \t missing table entry for $k\n";
        }
    }
}

# Depreciated
sub parseSummary {
    my $input = shift;
    my $line;
    my @words;
    my $values;
    my $i;

    open(IN, $input) || die("can't open file");

    while ($line=<IN>) {
        $line =~ s/\"//g;
        chomp($line);
        @words = split(/,/, $line);
        $values = [];   #creates a new anonymous array
        for( $i=1; $i<@words; $i++ )
        {
            push (@$values, $words[$i]);
        }
        $hash_enums{$words[0]} = $values;   # add
        push @{ $hash_types{$words[1]} }, $words[0];
        push (@parseOrder, $words[0]);
    }
    close(IN);
}
