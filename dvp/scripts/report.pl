#!/usr/bin/perl -w

# Copyright (C) 2009-2011 Texas Instruments, Inc.
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
# v02   J.Villarreal    - Migrating to latest DVP output structure.
#                       - Pruning out performance from cores that don't belong to the test core.
#                       - Dividing by 1000 to give performance in terms of ms.
# v03   J.Villarreal    - Migrating to latest Performance printout structure.
#                       - Changing imx to simcop
# v04   J.Villarreal    - Migrating to use new multiple header file configration
#                         from FOSS refactoring.
# v05   J.Villarreal    - Adding argument for log name

$argc = @ARGV;
if ($argc < 1) {
  die "\nusage:   perl report.pl <log file name> <dvp_types.h file> <top_directory_name> <0:debug/1:release> <num runs> <additional header files>\nversion: v04\n\n";
}

my $log_name = shift;
my $header = shift;
my $top_dir = shift;
my $release = shift;
my $runs = shift;
my $report  = "$top_dir/report_$log_name.txt";
my @stArray;
my %stHash;
my %libHash;
my %featHash;
my %tmpHash;
my @num;
my @core;
my @kernel;
my @first;
my @avg;
my @test;
my $cnt;
my @core_dir;
my $test_core;
my @headers;

# Parse all the library specific header files from command line
for( $k=6; $k<=$argc; $k++ ) {
    push (@headers, shift);
}

if(-d $top_dir) {
    open(REP, ">$report");
}
else {
    die "$top_dir does not exist.\n";
}

# When we only want to put the performances of the core whose directory this is in,
#   we need to know the name of the core for screening later.
@core_dir = split(/\// ,$top_dir);
$test_core = $core_dir[@core_dir-1];

if($release) {
    #@directories = ('qqvga', 'qvga', 'vga', '4xvga');  # OLD VERSION
    @directories = ('qqvga', 'qvga', 'vga');
    foreach $dir (@directories) {
        $dir = $top_dir . '/' . $dir;
    }
} else {
    @directories = <$top_dir/*>;
}

# Parse dvp_types.h
%featHash = %{&parseHeader($header, "_KF_", "^typedef enum _dvp_kfeat_e", "^} DVP_KernelFeature_e")};
%libHash = %{&parseHeader($header, "_KL_", "^typedef enum _dvp_klib_e", "^} DVP_KernelLibrary_e")};
%stHash = %{&parseHeader($header, "_KN_", "^typedef enum _dvp_knode_e", "^} DVP_KernelNode_e")};

# Parse library specific DVP header files
foreach $k (@headers) {
    %tmpHash = %{&parseHeader($k, "_KN_", "^enum ", "^};")};
    @stHash{ keys %tmpHash } = values %tmpHash;                 #concatenate hashes
}

$dirCnt = 0;

foreach $dir (@directories) {
    if(-d $dir) {

        my $infile = "$dir/log_$log_name";
        my $out = "$dir/log_$log_name.txt";
        my $newAvg;
        my $line;
        my @words;
        my @sub_dir;

        $cnt = 0;
        $active = 0;

        open(IN, $infile) || die("can't open file");
        open(OUT, ">$out") || die("can't open file");

        while ($line=<IN>) {
            if($line =~ /Node Performance/) {
                $active = 1;
            }
            elsif ($line =~ /Segmentation fault/) {
                print "    ERROR: $infile has a Segmentation Fault\n";
            }

            if($active)
            {
                chomp($line);
#               $line =~ s/D\/DVP_ZONE_PERF\( \d+\)://;     # OLD VERSION
                $line =~ s/DVP_ZONE_PERF://;
                $line =~ s/ //g;
                @words = split(/,/, $line);
                if (@words > 6 && $words[2] =~ /0x/) {
                    $words[2] =~ s/0x//;
                    $line =~ s/0x$words[2]/$stHash{hex($words[2])}/;
                    if($words[1] =~ /simcop/) {
                        $newAvg = &restrict_num_decimal_digits(($words[6]-$words[4])/($runs-1)/1000,3);
                    }
                    else {
                        $newAvg = &restrict_num_decimal_digits($words[5]/1000,3);
                    }
                    $num[$dirCnt][$cnt] = $words[0];
                    $core[$dirCnt][$cnt] = $words[1];
                    $kernel[$dirCnt][$cnt] = $stHash{hex($words[2])};
                    $first[$dirCnt][$cnt] = $words[4];
                    $avg[$dirCnt][$cnt] = $newAvg;
#                   if($core[$dirCnt][$cnt] !~ /m3/) {
#                       $avg[$dirCnt][$cnt] = 0;
#                   }
                    @sub_dir = split(/\// ,$dir);
                    #print "\n\n$sub_dir[1]\n";
                    $test[$dirCnt] = $sub_dir[@sub_dir-1];
                    $cnt++;
                }
                $line =~ s/,/\t/g;
                $line = $line . "\n";
                print OUT $line;
            }
        }

        if($active == 0)
        {
            print "    ERROR: $infile does not have performance data\n";
        }

        close(IN);
        close(OUT);
        $dirCnt++;
    }
}

print REP "Num\tKernel\tCore";  foreach $i (@test)  {print REP "\t$i";} print REP "\n";
for ($i=0; $i<$cnt; $i++) {
    print REP "$num[0][$i]\t$kernel[0][$i]\t$core[0][$i]";
    for ($j=0; $j<$dirCnt; $j++) {
        if ($avg[$j][$i] &&
           ($core[$j][$i] eq $core[0][$i]) &&
           ( ($core[$j][$i] =~ /$test_core/) || !($release) )
           ) {
            print REP "\t$avg[$j][$i]"
        } else {
            print REP "\t"
        }
    }
    print REP "\n";
}

close(REP);


# Parses a header file and returns a hash of enums/values
sub parseHeader {
    my $header = shift;     # Name of file to parse
    my $searchStr = shift;  # Search string
    my $start = shift;      # Starting delimeter in file
    my $end = shift;        # Ending delimeter in file
    my %hash = ();          # Start a new hash

    # Taken directly from dvp_types.h (easier than parsing, since it won't change)
    my $DVP_KN_RANGE = hex(1000);
    my $DVP_KN_FEATURE_START = hex(0);
    my $DVP_KN_LIBRARY_START = hex(40000000);

    open(HEAD, $header) || die("can't open file: $header");

    $active = 0;
    $value = 0;
    while ($line=<HEAD>) {
        if($line =~ /$start/) {
            $active = 1;
        }
        elsif ($line =~ /$end/) {
            $active = 0;
        }

        if($active)
        {
            if($line =~ /=\s+(\d+),/) {             # For when an enum is = a decimal value
                $value = $1;
            }
            elsif($line =~ /=\s+0x([0-9A-F]+),/) {  # For when an enum is = a hex value
                $value = hex($1);
            }
            elsif($line =~ /=\s+DVP_KN_FEATURE_BASE\((DVP_KF_\w+)\),/) {
                $value = $DVP_KN_FEATURE_START + ($featHash{$1} * $DVP_KN_RANGE);
            }
            elsif($line =~ /=\s+DVP_KN_LIBRARY_BASE\((DVP_KL_\w+)\),/) {
                $value = $DVP_KN_LIBRARY_START + ($libHash{$1} * $DVP_KN_RANGE);
            }

            if($line =~ /\s+(\w+$searchStr\w+),?/) {
                if($searchStr eq "_KN_") {
                    $hash{$value} = $1;
                } else {
                    $hash{$1} = $value;
                }
                $value++;
            }
        }
    }
    close(HEAD);
    return \%hash;
}

sub round {
  my $number = shift || 0;
  my $dec = 10 ** (shift || 0);
  return int( $dec * $number + .5 * ($number <=> 0)) / $dec;
}

# restrict the number of digits after the decimal point
sub restrict_num_decimal_digits
{
  my $num=shift;#the number to work on
  my $digs_to_cut=shift;# the number of digits after
                # the decimal point to cut
        #(eg: $digs_to_cut=3 will leave
        # two digits after the decimal point)

  if ($num=~/\d+\.(\d){$digs_to_cut,}/)
  {
    # there are $digs_to_cut or
    # more digits after the decimal point
    $num=sprintf("%.".($digs_to_cut-1)."f", $num);
  }
  return $num;
}
