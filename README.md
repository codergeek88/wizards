# wizards
write a database of harry potter wizards (and witches) using C

wizards.c implements a basic database with the four CRUD operations
the entries in the database are meant to be wizards and witches with the following information:
  first name
  last name
  hogwarts house
  alive (1 if alive, 0 if dead)
  good (1 if good, 0 if evil)
  first name slot
  last name slot
 
the first name and last name slots indicate the index in the database where the wizard's information will be stored/where it can be accessed
therefore, when a user looks up a first name or a last name of a wizard, they will be redirected to that index

collisions are handled by shifting information over by 1 index
  so if there already is a wizard stored at the desired index, the data will be stored at the next available index

if you're a harry potter fan, feel free to try to add some of your own witches and wizards!
