# **Motivations** #
My name is Christophe Hoël and as a diesel engine calibration engineer, I very often deal with Asap and Hex files. In my all day work, I use Inca from Ethas but since I started my career I have looked for an easy, quick and free tool to deal with Hex files.
Having no other solution, I decided in 2004 to start writing by myself an Asap file parser and Hex file editor in Matlab. Then I switched to C# to finish programming a final solution into C++.

I am definitively not a software development engineer but a mechanical engineer and I think the best way to learn is "learning by doing".
I spent lots of my free time programming this tool. So please be cool enough to respect the work done and share useful critics.

[my LinkedIn profile](http://ch.linkedin.com/pub/christophe-hoël/30/b14/385/)


# **Description** #
Most ECU (Engine Control Unit) in automotive use HEX file format.
The HEX file contains the software and the calibration labels to run the engine properly.

The A2L file (description file) is a text file used to describe the contain of the HEX file.
Its format is defined into the [ASAM](http://www.asam.net) standards as ASAM MCD-2 MC file (ASAP2).

HEXplorer is fast, robust and safe.
You first create a new project importing an A2L file. An A2L file with 1'750'000 lines containing around 40'000 calibration labels is parsed and imported into HEXplorer within 3s.
Then, you can add as many as wanted HEX files to your new created project.

HEXplorer allows you to edit the calibration values contained into the HEX file in physical format.
You can compare HEX files, edit calibration values, plot and so on...

Once the modifications done, you can save the modified HEX file.

# **Warnings** #
HEXplorer is only an ASAP file parser and HEX file editor. Nothing else!

You will never get any ASAP or HEX files with HEXplorer. It is your own responsibility to get the ASAP and HEX file to be parsed and edited into HEXplorer.

I definitely do not spread any information concerning ASAP and HEX files from automotive manufacturer. It make no sense for me, I have no interest and overall it is illegal.

# **News** #
HEXplorer new official release for Win32

  * 02 September 2012 Version 0.6.11 released
  * 15 March 2012 Version 0.6.10 released
  * 03 Dec 2011 Version 0.6.9 released
  * 17 Nov 2011 Version 0.6.8 released
  * 19 Oct 2011 Version 0.6.7 released (automatic update check)
  * 9 Oct 2011 Version 0.6.5 released
  * 15 Aug 2011 Version 0.4.23 released

# **Downloads** #
Use "Featured Downloads" on the right hand side. For more options and information, go to the [Downloads](http://code.google.com/p/hexplorer/wiki/Downloads) tab.

svn repository

# **Related projects** #
  * [Qt](http://qt.nokia.com/products/)
  * [Quex](http://quex.sourceforge.net/)
  * [QScintilla](http://www.riverbankcomputing.co.uk/software/qscintilla/download)
  * [Qwt](http://qwt.sourceforge.net/)
  * [Qwtplot3D](http://qwtplot3d.sourceforge.net/)
  * [Winhoard](http://www.hoard.org/) (only for windows)

# **To be done...** #
  * full ASAM3 norm implementation
  * Documentation
  * Wiki