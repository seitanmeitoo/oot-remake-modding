# Cinema

Des outils pour tourner des plans de coupe dans le remake Switch 2 d'Ocarina of Time, sur
[KaeGae Core](../../api/README.md) :

- une caméra libre, de 0,1 à 60 m/s, avec sa vitesse de rotation réglable ;
- le monde figé ou au ralenti, Link compris, pendant que la caméra continue de voler ;
- la téléportation vers n'importe quelle zone, donjon ou salle de boss, et son point d'arrivée ;
- Link : son âge (enfant ou adulte), son épée, son bouclier, sa tunique et ses bottes, sa
  position sauvée et rechargée, la vie infinie, et Link porté par la caméra pour le poser où on veut ;
- un panneau cliquable à la souris pour tout ça, plus la page Cinema du menu des mods (− et +)
  et des raccourcis manette modifiables.

**Licence : GPL-3.0-or-later** ([LICENSE](LICENSE)), parce que la téléportation vient de
sloperinaGZ, qui l'est.

**État, 19 novembre 2026 : version 0.5.0.** Jeu 1.1.1 uniquement : sur une autre version, le mod
ne fait rien. La caméra, le temps et le panneau (0.3.0) marchent dans Fritchii v6.1 avec KaeGae
Core 0.8.5. La téléportation (0.4.0) et l'onglet Link (0.5.0) sont à tester.

## Installation

Il faut KaeGae Core 0.8 ou plus récent :

```
python3 tools/kaegae.py install releases/kaegae-core.zip build/cinema.zip
```

Pour le panneau, la souris de l'émulateur doit être activée (dans Fritchii : réglages des
contrôles, onglet Avancé, case Souris).

## Utilisation

- **Panneau souris** : L + R, ou l'action « Panneau souris » de la page Cinema. Une touche du
  clavier peut lui être donnée sur cette page. Trois onglets : « Caméra et temps »,
  « Téléportation » et « Link ». Le même raccourci ferme le panneau, pour qu'il ne soit pas dans l'image.
- **Caméra libre** : pas de raccourci par défaut (à donner sur la page Cinema). Stick gauche pour avancer, stick droit pour regarder, ZR et ZL pour monter
  et descendre, croix droite et gauche pour changer de vitesse, A pour revenir à la caméra du jeu.
- **Temps** : figer et ralentir depuis le panneau ou la page Cinema ; raccourcis à donner soi-même.
- **Téléportation** : choisir l'onglet, le lieu puis le point d'arrivée, et cliquer sur « Y aller ».
  Le monde reprend sa vitesse et la caméra libre se coupe avant de partir.
- **Link** : changer d'âge recharge le lieu (dans sa version enfant ou adulte). L'équipement
  proposé dépend de l'âge. « Déplacer Link » demande la caméra libre : Link la suit, un mètre
  derrière, et reste où il est quand on le lâche. La position sauvée ne se recharge que dans le même lieu.

Tous les raccourcis se changent sur la page Cinema du menu des mods.

## Ce qui vient d'ailleurs

- **Caméra libre** : le mode vol libre de [FPV Drone](../fpv-drone/README.md) (MIT,
  KaboWabo/Kabosune), sans les modes drone.
- **Lecture de la souris et curseur** : [Mouse Grab](../mouse-grab/README.md) (MIT,
  KaboWabo/Kabosune).
- **Vitesse du temps du jeu** : trouvée par le mode photo de Lichii (zortos293/Lichii, PR #618).
- **Téléportation, âge, équipement et vie infinie** : [sloperinaGZ](https://github.com/PracticeROM/sloperina) 0.0.4.2
  de PracticeROM et ses contributeurs (GPL-3.0-or-later), par
  [Slopped Up Sloperina](../slopped-up-sloperina/README.md), son portage sur KaeGae Core.

## Modifications par rapport à sloperinaGZ

La GPL demande de signaler les modifications.

- `warp.c` : le `warp.c` de sloperinaGZ et les adresses 1.1.1 de son `z64hd_builds.h`. Modifié :
  jeu 1.1.1 seulement (adresses en constantes, GameData par `gamedata.c`), délais comptés sur l'horloge système au lieu du compteur d'images du jeu,
  réservation tardive de notre propre lieu comptée comme réussie, messages en français
  pour le panneau au lieu des notifications de gz, option des cinématiques d'arrivée retirée
  (toujours l'arrivée standard), `warp_pending` et `warp_abandoned` ajoutés pour l'âge.
- `gamedata.c` : les parties String64 et Bool du `gamedata.c` de sloperinaGZ. Modifié : jeu 1.1.1
  seulement, mémoire vérifiée par `kae_readable` au lieu de `sys.c`, Int, Float et Enum retirés.
- `link.c` : le `age.c` de sloperinaGZ (âge et rechargement du lieu), `link_actor`, `link_health` et
  la recherche du composant d'équipement de son `link.c`, la façon de retirer un équipement de son
  `pouch.c` (le champ String64, puis « tout équiper ») et le parcours de la mémoire de son `sys.c`.
  Modifié : jeu 1.1.1 seulement, équipement mis aussi bien que retiré à partir d'une liste par âge,
  recherche dans la mémoire au plus une fois toutes les 10 s après un échec, rafraîchissement de
  l'affichage de l'écran des objets retiré, messages en français.
- `places.c` : généré depuis les tables `data_warp_maps` et `data_spawns` du `data.c` de
  sloperinaGZ. Modifié : noms des lieux traduits en français ; les points d'arrivée gardent leurs
  noms anglais.

## Compilation

```
python3 tools/kaemod.py mods/cinema --no-core --out build/cinema.zip
```
