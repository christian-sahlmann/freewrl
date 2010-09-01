There are -two- types of version files here, a) release versions and b) library versions.


A: Release versions

- These files should contain a release number, in the expected usual format
(ie:  1.22.9 )



B: Library versions

- These files end with _LTVERSION, and then contain a string of the form "current[:revision[:age]]" (all integers)

- Update the version information only immediately before a public release of your software. More frequent updates are
  unnecessary, and only guarantee that the current interface number gets larger faster.

- If the library source code has changed at all since the last update, then increment revision 
  (‘c:r:a’ becomes ‘c:r+1:a’).  (this is what we do most of the time)

- If any interfaces have been added, removed, or changed since the last update, increment current, and set revision to 0.

- If any interfaces have been added since the last public release, then increment age.

- If any interfaces have been removed or changed since the last public release, then set age to 0. 
