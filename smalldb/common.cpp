#include "common.hpp"

#include <iostream>

bool read_exact(int fd, void *buffer, int nbytes)
{
   int lu, i = 0;
   while (i < nbytes && (lu = read(fd, buffer, nbytes - i)) > 0)
   {
      i += lu;
   }

   if (lu < 0)
   {
      std::cerr << ("read()");
   }

   return lu > 0;
}
