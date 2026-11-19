# Cinema

Des outils pour tourner des plans de coupe dans le remake Switch 2 d'Ocarina of Time, sur
[KaeGae Core](../../api/README.md) :

- une caméra libre, de 0,1 à 60 m/s, avec sa vitesse de rotation réglable ;
- le monde figé ou au ralenti, Link compris, pendant que la caméra continue de voler ;
- la téléportation vers n'importe quelle zone, donjon ou salle de boss, et son point d'arrivée ;
- Link : son âge (enfant ou adulte), son épée, son bouclier, sa tunique et ses bottes, sa
  position sauvée et rechargée, la vie infinie, et Link porté par la caméra pour le poser où on
  veut ;
- l'inventaire (objets, équipement, masques, objets d'échange, munitions, objets de quête, chansons)
  et l'étape de chacune des 35 quêtes, avec « Tout terminer » et « Tout au début » ;
- un panneau cliquable à la souris pour tout ça, plus la page Cinema du menu des mods (− et +)
  et des raccourcis manette modifiables.

**Licence : GPL-3.0-or-later** ([LICENSE](LICENSE)), parce que la téléportation vient de
sloperinaGZ, qui l'est.

**État, 19 novembre 2026 : version 0.6.0.** Jeu 1.1.1 uniquement : sur une autre version, le mod
ne fait rien. La caméra, le temps et le panneau (0.3.0) marchent dans Fritchii v6.1 avec KaeGae
Core 0.8.5. La téléportation (0.4.0) et l'onglet Link (0.5.0) marchent ; l'équipement s'applique en
rechargeant le lieu. Les onglets Inventaire et Histoire (0.6.0) marchent aussi.

## Installation

Pour Windows, avec Fritchii, Eden ou Lichii. **Fermer l'émulateur avant.**

1. Télécharger `install-cinema.ps1` depuis la dernière
   [Release](https://github.com/seitanmeitoo/oot-remake-modding/releases/latest).
2. Dans PowerShell, dans le dossier du script :

   ```powershell
   powershell -ExecutionPolicy Bypass -File .\install-cinema.ps1
   ```

Le script trouve le dossier de mods de l'émulateur, télécharge `cinema-pack.zip` (KaeGae Core et
Cinema, rien d'autre) et l'installe en remplaçant une ancienne version. Si un autre mod occupe déjà
l'emplacement de Cinema, il en prend un libre. Options :

| Option | Effet |
|---|---|
| `-DesactiverLesAutres` | met les autres mods du jeu de côté (dans `load-desactives`, à côté de `load`) |
| `-ReactiverLesAutres` | les remet |
| `-Desinstaller` | retire Cinema, et KaeGae Core si plus aucun mod ne s'en sert |
| `-LoadDir <dossier>` | le dossier `load` de l'émulateur, s'il n'est pas trouvé tout seul |
| `-Pack <zip>` | un `cinema-pack.zip` déjà téléchargé |

À la main : décompresser `cinema-pack.zip` dans le dossier de mods de l'émulateur (dans Fritchii,
`%APPDATA%\fritchii\load\`). Depuis le dépôt, avec Python :
`python3 tools/kaegae.py install releases/kaegae-core.zip build/cinema.zip`.

Pour le panneau, la souris de l'émulateur doit être activée (dans Fritchii : réglages des
contrôles, onglet Avancé, case Souris).

## Publier une version

Changer `version` dans `mod.toml`, commiter, puis pousser un tag `cinema-v<version>` (par exemple
`cinema-v0.6.0`). Le workflow `.github/workflows/cinema-release.yml` compile Cinema, fait
`cinema-pack.zip` avec `releases/kaegae-core.zip`, et crée la Release avec le zip et le script.

## Utilisation

- **Panneau souris** : L + R, ou l'action « Panneau souris » de la page Cinema. Une touche du
  clavier peut lui être donnée sur cette page. Cinq onglets : « Caméra »,
  « Téléportation », « Link », « Inventaire » et « Histoire ». Le même raccourci ferme le panneau, pour qu'il ne soit pas dans l'image.
- **Caméra libre** : pas de raccourci par défaut (à donner sur la page Cinema). Stick gauche pour avancer, stick droit pour regarder, ZR et ZL pour monter
  et descendre, croix droite et gauche pour changer de vitesse, A pour revenir à la caméra du jeu.
- **Temps** : figer et ralentir depuis le panneau ou la page Cinema ; raccourcis à donner soi-même.
- **Téléportation** : choisir l'onglet, le lieu puis le point d'arrivée, et cliquer sur « Y aller ».
  Le monde reprend sa vitesse et la caméra libre se coupe avant de partir.
- **Link** : changer d'âge recharge le lieu (dans sa version enfant ou adulte). L'équipement
  proposé dépend de l'âge. « Déplacer Link » demande la caméra libre : Link la suit, un mètre
  derrière, et reste où il est quand on le lâche. La position sauvée ne se recharge que dans le même lieu.
- **Inventaire et Histoire** : ils modifient la sauvegarde, donc mieux vaut en utiliser une à part.
  Une quête changée se voit souvent après un rechargement du lieu.

Tous les raccourcis se changent sur la page Cinema du menu des mods.

## Limites connues

- **Jeu 1.1.1 seulement.** Les adresses du jeu sont écrites en dur pour cette version.
- **Équipement** : il s'applique en rechargeant le lieu, et Link réapparaît à sa dernière entrée.
  L'adresse de l'objet « équipement » reprise de sloperinaGZ ne correspond à rien en 1.1.1.
- **Retirer un objet** ne l'enlève ni de Link s'il le porte, ni du bouton d'objet où il est assigné
  (jusqu'à l'ouverture de l'écran des objets).
- **Ocarinas** non gérés : ils passent par une autre partie du jeu. Sans ocarina dans la sauvegarde,
  les chansons ajoutées ne peuvent pas être jouées.
- **Quêtes** : une étape changée se voit souvent seulement après un rechargement du lieu. Mettre une
  quête à une étape impossible peut bloquer un événement du jeu : utiliser une sauvegarde à part.
- **Position sauvée** : elle ne se recharge que dans le même lieu, et disparaît quand le jeu s'arrête.
- **Désactiver Cinema** dans le menu des mods pendant que le monde est figé le laisse figé :
  relancer le monde avant.
- **Libellés de la page Cinema du menu des mods** sans accents : la liste de Core ne les affiche pas
  (le panneau, lui, les affiche).
- **Pas de vitesse ×2** pour Link : la faire en déplaçant Link en plus du jeu le faisait revenir en
  arrière. Pour aller vite, « Déplacer Link » avec la caméra libre fait le même travail.

## Ce qui vient d'ailleurs

- **Caméra libre** : le mode vol libre de [FPV Drone](../fpv-drone/README.md) (MIT,
  KaboWabo/Kabosune), sans les modes drone.
- **Lecture de la souris et curseur** : [Mouse Grab](../mouse-grab/README.md) (MIT,
  KaboWabo/Kabosune).
- **Vitesse du temps du jeu** : trouvée par le mode photo de Lichii (zortos293/Lichii, PR #618).
- **Téléportation, âge, équipement, inventaire, quêtes et vie infinie** : [sloperinaGZ](https://github.com/PracticeROM/sloperina) 0.0.4.2
  de PracticeROM et ses contributeurs (GPL-3.0-or-later), par
  [Slopped Up Sloperina](../slopped-up-sloperina/README.md), son portage sur KaeGae Core.

## Modifications par rapport à sloperinaGZ

La GPL demande de signaler les modifications.

- `warp.c` : le `warp.c` de sloperinaGZ et les adresses 1.1.1 de son `z64hd_builds.h`. Modifié :
  jeu 1.1.1 seulement (adresses en constantes, GameData par `gamedata.c`), délais comptés sur l'horloge système au lieu du compteur d'images du jeu,
  réservation tardive de notre propre lieu comptée comme réussie, messages en français
  pour le panneau au lieu des notifications de gz, option des cinématiques d'arrivée retirée
  (toujours l'arrivée standard), `warp_pending` et `warp_abandoned` ajoutés pour l'âge.
- `gamedata.c` : le `gamedata.c` de sloperinaGZ sans les Float (types, recherche par hachage,
  murmur3, Int, Enum, String64 et Bool). Modifié : jeu 1.1.1 seulement, mémoire vérifiée par
  `kae_readable` au lieu de `sys.c`, Int lus directement en mémoire (pas par la fonction du jeu), car
  le panneau les lit depuis le dessin.
- `link.c` : le `age.c` de sloperinaGZ (âge et rechargement du lieu), `link_actor`, `link_health` et
  la recherche du composant d'équipement de son `link.c`, la façon de retirer un équipement de son
  `pouch.c` (le champ String64, puis « tout équiper ») et le parcours de la mémoire de son `sys.c`.
  Modifié : jeu 1.1.1 seulement, équipement mis aussi bien que retiré à partir d'une liste par âge,
  recherche dans la mémoire au plus une fois toutes les 10 s après un échec, rafraîchissement de
  l'affichage de l'écran des objets retiré, messages en français.
- `inventory.c` : `field_get` / `field_set`, la possession des objets, les objets un à la fois
  (masques, échange), les étapes des quêtes du `menus.c` de sloperinaGZ, et le remplissage des
  munitions de son `cheats.c`. Modifié : un objet retiré n'est enlevé ni de Link ni des boutons
  d'objet, « toutes les quêtes terminées ou au début », un équipement porté devient possédé.
- `inventory_data.c` : généré depuis les tables d'objets, d'équipement, de masques, d'échange, de
  munitions, d'objets de quête, de chansons et de quêtes du `data.c` de sloperinaGZ. Modifié : noms
  traduits en français ; les étapes des quêtes gardent les noms du jeu.
- `places.c` : généré depuis les tables `data_warp_maps` et `data_spawns` du `data.c` de
  sloperinaGZ. Modifié : noms des lieux traduits en français ; les points d'arrivée gardent leurs
  noms anglais.

## Compilation

```
python3 tools/kaemod.py mods/cinema --no-core --out build/cinema.zip
```
