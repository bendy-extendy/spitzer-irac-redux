#!/usr/bin/perl

@foo = ` ls */Ch?/cdf/overlap.nl`;
chomp(@foo);
foreach(@foo)
  {
   print "Editing $_\n";
   $command = "cp -r $_  $_" . "_old";
   $foo = `$command`;
   $command = 'sed "s/INTERP_METHOD = 3,/INTERP_METHOD = 2,/" < ' . $_ . '> temp';
   $foo = `$command`;
   $foo = `mv -f temp $_`;
  }
 
