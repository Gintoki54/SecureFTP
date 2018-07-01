# SecureFTP
## Fonctionnement
Pour le faire fonctionner il suffit de lancer le serveur:
```
./server
```
Ensuite il faut lancer le client, dans un premier temps, il
faut créer un compte avec la commande suivante:
```
./client -c
```
Le programme vous demandera de saisir un login et un mot de passe.
Ces deux informations seront envoyées au serveur qui va hasher le 
mot de passe et le stocker sous la forme suivante:
`<login>:<SHA256 de votre mot de passe>`

Ensuite vous pouvez envoyer un fichier au serveur:
```
./client -n <login> -p <password> -f <chemin_du_fichier>
```
Ce fichier va être enregistré dans le dossier `./Server/<login>`

Je reste disponible pour éventuel entretien afin de discuter du 
projet afin d'avoir vos retours.
