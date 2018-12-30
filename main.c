#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

char *progname;
char *inFile;
char *outFile;
bool inFileSet;
bool outFileSet;

typedef struct {
   char *nachname;
   char *vorname;
   char *strasse;
   char *ort;
   char *plz;
   char *iban;
   char *bic;
} VR_kontakt;

typedef struct {
   char *name;
   char *strasse;
   char *ort;
   char *iban;
   char *bic;
} Proficash_kontakt;

void help() {
   puts("Usage:");
   printf("%s -i INPUT_FILE -o OUTPUT_FILE\n", progname);
   puts("Order of options can be changed.");
   puts("INPUT_FILE  : File with contact data in VRNetworld-format.");
   puts("OUTPUT_FILE : Where to save the converted output compatible with proficash import.");
   puts("INPUT_FILE must exist, else the program will abort.");
   puts("OUTPUT_FILE must not exist, since the proram will not override existing data.");
}

int getVRContacts(VR_kontakt **vr_kontakte, FILE *file) {
   int size = 0;
   
   do {
      size++;
      *vr_kontakte = realloc(*vr_kontakte, sizeof(VR_kontakt) * size);
   } while(fscanf(file,
		  "%m[^;];%m[^;];%m[^;];%m[^;];%m[^;];%m[^;];%m[^\n]\n",
		  &(((*vr_kontakte)[size-1]).nachname),
		  &(((*vr_kontakte)[size-1]).vorname),
		  &(((*vr_kontakte)[size-1]).strasse),
		  &(((*vr_kontakte)[size-1]).ort),
		  &(((*vr_kontakte)[size-1]).plz),
		  &(((*vr_kontakte)[size-1]).iban),
		  &(((*vr_kontakte)[size-1]).bic)
		  ) == 7
	   );
   size--;
   *vr_kontakte = realloc(*vr_kontakte, sizeof(VR_kontakt) * size);

   return size;
}

void printVRContact(VR_kontakt contact) {
   printf("Vorname: %s\n", contact.vorname);
   printf("Nachname: %s\n", contact.nachname);
   printf("Strasse: %s\n", contact.strasse);
   printf("Ort: %s\n", contact.ort);
   printf("PLZ: %s\n", contact.plz);
   printf("IBAN: %s\n", contact.iban);
   printf("BIC: %s\n", contact.bic);
}

bool getProficashContacts(VR_kontakt *vr_contacts, int count, Proficash_kontakt **proficash_contacts) {
   *proficash_contacts = calloc(count, sizeof(Proficash_kontakt));
   for(int i = 0; i < count; i++) {
      (*proficash_contacts)[i].name = malloc(strlen(vr_contacts[i].vorname) + strlen(vr_contacts[i].nachname) + 3);
      sprintf((*proficash_contacts)[i].name, "%s, %s", vr_contacts[i].nachname, vr_contacts[i].vorname);
      (*proficash_contacts)[i].strasse = malloc(strlen(vr_contacts[i].strasse) + 1);
      strcpy((*proficash_contacts)[i].strasse, vr_contacts[i].strasse);
      (*proficash_contacts)[i].ort = malloc(strlen(vr_contacts[i].ort) + strlen(vr_contacts[i].plz) + 2);
      sprintf((*proficash_contacts)[i].ort, "%s %s", vr_contacts[i].plz, vr_contacts[i].ort);
      (*proficash_contacts)[i].iban = malloc(strlen(vr_contacts[i].iban) + 1);
      strcpy((*proficash_contacts)[i].iban, vr_contacts[i].iban);
      (*proficash_contacts)[i].bic = malloc(strlen(vr_contacts[i].bic) + 1);
      strcpy((*proficash_contacts)[i].bic, vr_contacts[i].bic);
   }
   return true;
}

void printProficashContact(Proficash_kontakt contact) {
   printf("Name: %s\n", contact.name);
   printf("Strasse: %s\n", contact.strasse);
   printf("Ort: %s\n", contact.ort);
   printf("IBAN: %s\n", contact.iban);
   printf("BIC: %s\n", contact.bic);
}

int writeProficashContacts(Proficash_kontakt *contacts, int count, FILE *file) {
   int i = 0;
   while(i < count && fprintf(file, "%s;%s;%s;%s;%s\n", contacts[i].name, contacts[i].strasse, contacts[i].ort, contacts[i].iban, contacts[i].bic) >= 0) {
      i++;
   }
   return i;
}

void destroyVRContacts(VR_kontakt *contacts, int count) {
   for(int i = 0; i < count; i++) {
      free(contacts[i].vorname);
      free(contacts[i].nachname);
      free(contacts[i].strasse);
      free(contacts[i].ort);
      free(contacts[i].plz);
      free(contacts[i].iban);
      free(contacts[i].bic);
   }
   free(contacts);
}

void destroyProficashContacts(Proficash_kontakt *contacts, int count) {
   for(int i = 0; i < count; i++) {
      free(contacts[i].name);
      free(contacts[i].strasse);
      free(contacts[i].ort);
      free(contacts[i].iban);
      free(contacts[i].bic);
   }
   free(contacts);
}

int main(int argc, char *argv[]) {
   inFile = NULL;
   outFile = NULL;
   inFileSet = false;
   outFileSet = false;
   progname = argv[0];

   /* Fetch arguments and check their values */
   for (int arg; (arg = getopt(argc, argv, "i:o:")) != -1;) {
      switch (arg) {
      case 'i':
	 if(!(access(optarg, R_OK) == 0)) {
	    perror("Input file cannot be accessed");
	    return EXIT_FAILURE;
	 }
	 inFile = malloc(sizeof(char) * (strlen(optarg) + 1));
	 strcpy(inFile, optarg);
	 inFileSet = true;
	 break;
      case 'o':
	 outFile = malloc(sizeof(char) * (strlen(optarg) + 5));
	 strcpy(outFile, optarg);
	 strcat(outFile, ".csv");
	 if(access(outFile, F_OK) == 0) {
	    errno = EINVAL;
	    perror("Output file already exists");
	    return EXIT_FAILURE;
	 }
	 outFileSet = true;
	 break;
      default: // Invalid option
	 perror("Option not supported");
	 return EXIT_FAILURE;
      }
   }
   if(!(inFileSet && outFileSet)) {
      puts("Missing option!");
      return EXIT_FAILURE;
   }
   
   puts("Config:");
   printf("Input file  : %s\n", inFile);
   printf("Output file : %s\n", outFile);

   FILE *inFileHandle = fopen(inFile, "r");
   VR_kontakt *vr_kontakte = NULL;
   int size = 0;
   
   if((size = getVRContacts(&vr_kontakte, inFileHandle)) == 0) {
      perror("Error parsing input file");
      return EXIT_FAILURE;
   }

   Proficash_kontakt *proficash_kontakte = NULL;
   if(!(getProficashContacts(vr_kontakte, size, &proficash_kontakte))) {
      puts("Error during conversion.");
      return EXIT_FAILURE;
   }

   FILE *outFileHandle = fopen(outFile, "w");
   int writecount = 0;
   if((writecount = writeProficashContacts(proficash_kontakte, size, outFileHandle)) != size) {
      printf("Error during writing, only wrote %d values instead of %d\n", writecount, size);
      return EXIT_FAILURE;
   }

   destroyVRContacts(vr_kontakte, size);
   destroyProficashContacts(proficash_kontakte, size);
   fclose(outFileHandle);
   fclose(inFileHandle);
   free(inFile);
   free(outFile);
   return EXIT_SUCCESS;
}
