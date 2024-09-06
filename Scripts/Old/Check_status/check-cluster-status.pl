#!/usr/bin/perl


$allsched= "http://ssc.spitzer.caltech.edu/warmmission/scheduling/observinglogs/plan/alltogether.txt";
$alllog = "http://ssc.spitzer.caltech.edu/warmmission/scheduling/observinglogs/obs/alltogether.txt";

# Warm mission plan is now concatenated
$current = $startnum;

@file = ($allsched, $alllog);
@newname = ("sched.txt", "log.txt");
@outfile = ("Rigby_scheduled.txt", "Rigby_logged.txt");


for ($ii=0; $ii<=$#file; $ii++)
{
    open(OUT, ">$outfile[$ii]") or die "cannot write outfile $outfile[$ii]\n";
    $error = system("wget -N $file[ii] -O temp ");  #debugging, commenting out this crucial step
    print "status was $error (0=success)\n";
    system("tr -c '\11\12\40-\176' '\n' < temp > $newname[$ii]");  # strip stupid ^M chars

    @foo = `grep MASSZ $newname[0]`;  #MASSZ is the name of our program
    print OUT @foo;
    close(OUT);
    print "Echoing results, saved to $outfile[$ii]:\n" ;
    system("cat $outfile[$ii]");

}


