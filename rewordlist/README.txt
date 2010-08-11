===============================================================================
          +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+
          | R | | E | | W | | O | | R | | D | | L | | I | | S | | T |
          +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+
===============================================================================

By Al McLuckie
Any comments/suggestions to al-at-purplepup-dot-org


Synopsis:
=========
Rewordlist is the utility program that supports the reword game and generates the
required word file used by it.

License:
========
See source files.

Installing:
===========
reword_v0.4.zip unzips to reword_v0.4 directory and contains source code for reword game
and rewordlist utility.
Rewordlist compiles on Linux using make in the rewordlist directory, or by using 
the VC6 rewordlist.dsp on Win.

Instructions:
=============
Run rewordlist with --help for params.

Information:
============
The text file rewordlist.txt is the word file used in the reword game. Each line may
contain (separated by pipes '|') a word, an optional difficulty value (1=easy, 2=med, 3=hard) 
and definition. 
If the definition is missing, it can be filled in by running the worlist past an xdxf file, 
otherwise it will simply not give a definition in the game.
If the level is missing or invalid (not 1..3), it is assumed to be a easy level.
e.g. variations of valid line formats
ABBACY|2|(n.) The dignity, estate, or jurisdiction of an abbot.
ABBACY|2
ABBACY

The rewordlist utility can use the xdxf xml(ish) dictionary format to populate words 
with dictionary definitions. Dictionaries for various subjects and languages can be 
obtained from :
http://xdxf.sourceforge.net/

The word difficulty is just my interpretation of the available words with a mind on 
who will be using the game. I judged the following as:

Easy (1):    to be words that anyone (including kids) with a reasonable level of English 
             might know. Also this level will (eventually) have no swear words or offensive 
             slang. Currently only the 6 letter words are filtered for filth.

Medium (2):  might have more 'difficult' words, but still words in common use. I expect 
             this will be most peoples level of choice (and it's the default).
             May contain some slang words.

Hard (3):    is for words I feel are less likely to be in peoples vocabulary or difficult 
             to spell, or are names of people, animals or plants or may just be more obscure.
             This level may also contain offensive and swear words.

Note, words shorter than 6 letters can also have a difficulty level value and may eventually 
be blocked too. I have currently just done the potentially offensive 6 letter words, but 
I cannot guarantee all words are tagged as such.

Remember this is just my initial take on the wordlist and can be moderated by you or 
anyone creating a new wordlist. Also, remember that in easy mode, only easy words are used. 
In medium mode, all easy and medium words are used; in hard mode all easy, medium and 
hard words are used.

customising:
------------
To create your own rewordlist.txt game word file, run rewordlist with either a wordlist 
file (with a single word per line) or a xdxf dictionary file, or both! This will generate
the rewordlist.txt file that the game can use.

Using only a wordlist as the initial word source allows you to more easily 
control the words to be used in the game. You can then apply any big enough 
xdxf dictionary to fill in the definitions.

Using only the xdxf as the word source gives a potentially huge word list 
but may include words that are proper names etc, and not as good for a game

Using both a word list and a xdxf dictionary means you control the 6 letter wordlist and 
any assigned difficulty values, then apply any number of xdxf dictionaries to the list, 
producing the final game file

e.g. I started with the rewordlist_orig.txt file, ran it through rewordlist to produce 
rewordlist.txt. Renamed this to rewordlist_master.txt and added the difficulty values 
to the 6 letter words. Then used rewordlist_master.txt as the input and a xdxf dictionary
file for the definitions and used the final rewordlist.txt for the game. In fact I did more;
I reran each rewordlist.txt back through rewordlist with different xdxf dictionaries to try 
and fill in the gaps in the definitions. There are only a few gaps now which I might do manually.
Remember, you will need to change the output filename (of rewordlist.txt) each time you want 
to run it back through rewordlist as you can't use it as the input file in the next run, as it 
automatically calls the output filename rewordlist.txt (maybe I'll change that eventually).

If you want to remove a word (offensive etc) you can build a file with words you always want to 
exclude and name it with a ".exclude" extension. Likewise you can build a file with words that 
should always be added just in case they are not in the dictionary or are special to your wordlist. 
This should have the ".include" extension. The inclusion file can also have word level and 
dictionary definitions.

e.g. - mywords.include
CROXXX|3|A word I made up!
CROXXY|3|Another made up word


An example build run using multiple dictionaries:
//step1 - initial build
rewordlist inputwords.txt badwords.exclude musthave.include somedict.xdxf
//step2 - run through with another dictionary
mv rewordlist.txt tmp_rewordlist.txt
rewordlist tmp_rewordlist.txt someotherdict.xdxf



definitions:
------------
I now take the first 2000 characters of any xdxf definition, so most of the definition should be 
captured. If not, you will have to hand craft them ....


Version History:
================
0.4		18.03.2008	Added include/exclude word lists
0.3		30.05.2007	Added ability to parse and include xdxf dictionary definitions 
0.2		22.10.2006	Upped maximum words from 7 to 8 per 6 letter
0.1		06.09.2006	First version


If you have any problems or need the rewordlist source code changing to cope with a 
problem you encounter with other xdxf dictionaries, please let me know and I'll do my best.
My email address is at the top.

Thanks 
Al



