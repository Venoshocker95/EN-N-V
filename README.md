# EN.N.V.
EN.N.V. or ENIGMA NOTE VAULT, is a sotware based on the WW2 german ciphering machine Enigma.
The executable allows the user to:

° Create an encrypted note by ciphering user input *

° Decrypt a note that was encrypted with this same software *

° View a list of encrypted notes **

° Deleting a note from the list


I've tried to replicate Enigma's pecualiar functions in C, which has been even with all the resources
available on the web quite challenging (my poor head @ - @ )

*After the user has enter something to be encrypted or decrypted, the program will ask the user for
  3 alphabetical values, those will be the starting rotor positions.

** The rotor positions are also saved along the encrypted note, although they appear as numerical
   values ( just like the real deal), so to decypher you just need to know the alphabet albeit
   from "letter position"#0 to "letter position"#25 instead of the standard 1 to 26

RESOURCES USED:

° Wikipedia
° "The C Programming Language" by Brian W.Kernighan & Dennis M.Ritchie

° Numberphile Youtube Channel

° W3 Schools & Stack Overflow

° Microsoft Copilot (to explain difficoult concept, and helping when stuck while replicating
  Enigma's functionalities)
