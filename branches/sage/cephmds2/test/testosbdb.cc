/* testosbdb.cc -- test OSBDB.
   Copyright (C) 2007 Casey Marshall <csm@soe.ucsc.edu> */


#include <iostream>
#include "osbdb/OSBDB.h"

using namespace std;

int
main (int argc, char **argv)
{
  vector<char *> args;
  argv_to_vec (argc, argv, args);
  parse_config_options (args);

  g_conf.debug_bdbstore = 3;
  //g_conf.bdbstore_btree = true;
  char dbfile[256];
  strncpy (dbfile, "/tmp/testosbdb/db.XXXXXX", 256);
  mktemp (dbfile);
  OSBDB *os = new OSBDB(dbfile);
  auto_ptr<OSBDB> osPtr (os);
  os->mkfs();
  os->mount();

  // Put an object.
  object_t oid (0xDEADBEEF00000000ULL, 0xFEEDFACE);

  cout << "sizeof oid_t is " << sizeof (oid_t) << endl;
  cout << "offsetof oid_t.id " << offsetof (oid_t, id) << endl;

  cout << sizeof (object_t) << endl;
  cout << sizeof (oid.ino) << endl;
  cout << sizeof (oid.bno) << endl;
  cout << sizeof (oid.rev) << endl;

  // Shouldn't be there.
  if (os->exists (oid))
    {
      cout << "FAIL: oid shouldn't be there " << oid << endl;
    }

  // Write an object.
  char *x = (char *) malloc (1024);
  memset(x, 0xaa, 1024);
  bufferptr bp (x, 1024);
  bufferlist bl;
  bl.push_back (bp);

  if (os->write (oid, 0L, 1024, bl, NULL) != 1024)
    {
      cout << "FAIL: writing object" << endl;
    }

  os->sync();

  // Should be there.
  if (!os->exists (oid))
    {
      cout << "FAIL: oid should be there: " << oid << endl;
    }

  memset(x, 0, 1024);
  if (os->read (oid, 0, 1024, bl) != 1024)
    {
      cout << "FAIL: reading object" << endl;
    }

  for (int i = 0; i < 1024; i++)
    {
      if ((x[i] & 0xFF) != 0xaa)
        {
          cout << "FAIL: data read out is different" << endl;
          break;
        }
    }

  // Set some attributes
  if (os->setattr (oid, "alpha", "value", strlen ("value")) != 0)
    {
      cout << "FAIL: set attribute" << endl;
    }
  if (os->setattr (oid, "beta", "value", strlen ("value")) != 0)
    {
      cout << "FAIL: set attribute" << endl;
    }
  if (os->setattr (oid, "gamma", "value", strlen ("value")) != 0)
    {
      cout << "FAIL: set attribute" << endl;
    }
  if (os->setattr (oid, "fred", "value", strlen ("value")) != 0)
    {
      cout << "FAIL: set attribute" << endl;
    }

  char *attrs = (char *) malloc (1024);
  if (os->listattr (oid, attrs, 1024) != 0)
    {
      cout << "FAIL: listing attributes" << endl;
    }
  else
    {
      char *p = attrs;
      if (strcmp (p, "alpha") != 0)
        {
          cout << "FAIL: should be \"alpha:\" \"" << p << "\"" << endl;
        }
      p = p + strlen (p) + 1;
      if (strcmp (p, "beta") != 0)
        {
          cout << "FAIL: should be \"beta:\" \"" << p << "\"" << endl;
        }
      p = p + strlen (p) + 1;
      if (strcmp (p, "fred") != 0)
        {
          cout << "FAIL: should be \"fred:\" \"" << p << "\"" << endl;
        }
      p = p + strlen (p) + 1;
      if (strcmp (p, "gamma") != 0)
        {
          cout << "FAIL: should be \"gamma:\" \"" << p << "\"" << endl;
        }
    }

  coll_t cid = 0xCAFEBABE;
  if (os->create_collection (cid) != 0)
    {
      cout << "FAIL: create_collection" << endl;
    }
  if (os->create_collection (cid + 10) != 0)
    {
      cout << "FAIL: create_collection" << endl;
    }
  if (os->create_collection (cid + 5) != 0)
    {
      cout << "FAIL: create_collection" << endl;
    }
  if (os->create_collection (42) != 0)
    {
      cout << "FAIL: create_collection" << endl;
    }

  if (os->collection_add (cid, oid) != 0)
    {
      cout << "FAIL: collection_add" << endl;
    }

  list<coll_t> ls;
  if (os->list_collections (ls) < 0)
    {
      cout << "FAIL: list_collections" << endl;
    }
  cout << "collections: ";
  for (list<coll_t>::iterator it = ls.begin(); it != ls.end(); it++)
    {
      cout << *it << ", ";
    }
  cout << endl;

  if (os->destroy_collection (0xCAFEBABE + 10) != 0)
    {
      cout << "FAIL: destroy_collection" << endl;
    }

  if (os->destroy_collection (0xCAFEBADE + 10) == 0)
    {
      cout << "FAIL: destroy_collection" << endl;
    }

  object_t oid2 (12345, 12345);
  for (int i = 0; i < 8; i++)
    {
      oid2.rev++;
      if (os->collection_add (cid, oid2) != 0)
        {
          cout << "FAIL: collection_add" << endl;
        }
    }
  for (int i = 0; i < 8; i++)
    {
      if (os->collection_remove (cid, oid2) != 0)
        {
          cout << "FAIL: collection_remove" << endl;
        }
      oid2.rev--;
    }

  // Truncate the object.
  if (os->truncate (oid, 512, NULL) != 0)
    {
      cout << "FAIL: truncate" << endl;
    }

  // Expand the object.
  if (os->truncate (oid, 1200, NULL) != 0)
    {
      cout << "FAIL: expand" << endl;
    }

  // Delete the object.
  if (os->remove (oid) != 0)
    {
      cout << "FAIL: could not remove object" << endl;
    }

  // Shouldn't be there
  if (os->exists (oid))
    {
      cout << "FAIL: should not be there" << endl;
    }

  os->sync();
  exit (0);
}
