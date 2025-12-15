# 🎛️ Défi Final Info-Élec : Jeu du Servomoteur "Chaud-Froid"

Projet réalisé par **Timothée C.** & **Gabriel B.** dans le cadre du cours d'informatique-électronique (BAC1 ECAM).

## 🎯 Objectif du Jeu

Le but est de trouver une position angulaire "secrète" (choisie aléatoirement par l'Arduino) en orientant un servomoteur à l'aide d'un joystick.

Le joueur n'a pas d'indication visuelle directe de la cible, mais doit se fier à la couleur de la LED RGB qui indique si l'on se rapproche ou si l'on s'éloigne de la cible (principe du jeu "Chaud/Froid").

## 🕹️ Comment jouer ?

1.  **Lancer une partie** : Appuyez sur le bouton du joystick (ou attendez la fin de l'initialisation automatique).
2.  **Chercher l'angle** : Déplacez le joystick (Axe X) pour faire tourner le servomoteur.
3.  **Suivre les couleurs (Feedback)** :
    *   🔴 **Rouge** : Vous êtes très loin de la cible (Froid, erreur >= 40°).
    *   🟠 **Orange** : Vous vous rapprochez.
    *   🟡 **Jaune** : Vous êtes tout près !
    *   🟢 **Vert** : Angle trouvé ! (Erreur < 3°).
4.  **Gagner** : Maintenez la position dans la zone verte pendant **2 secondes**.
    *   🔵 **Bleu** : Victoire ! Une nouvelle partie recommence après une animation.
    *   ⚪ **Blanc** : Phase d'initialisation / attente.

## 🛠️ Schéma Électronique

Voici le montage réalisé sous KiCad :

![Schéma du circuit](Screenshot%20from%2025-12-14%2018-02-44.png)

### Liste du Matériel

*   **Microcontrôleur** : Arduino UNO.
*   **Actuateur** : Servomoteur (Modifié, voir détails ci-dessous).
*   **Contrôle** : Module Joystick (Axe X sur A1, Bouton sur D4).
*   **Affichage** :
    *   2x LEDs RGB (Anode commune).
    *   Diffuser lumineux maison (Boîtier, papier alu).
    *   Écran OLED SSD1306 128x64 pixels (via I2C).
*   **Électronique de puissance & protection** :
    *   3x Transistors **2N2219** (pour l'amplification des canaux R, V, B des LEDs).
    *   3x Résistances **330Ω** (limitation de courant des LEDs).
    *   3x Résistances **10kΩ** (résistances de base des transistors).
*   **Divers** : Fils de connexion, breadboard/PCB.

## ⚙️ Analyse Technique

### 1. Le "Hack" du Servomoteur (Feedback Réel)
Une particularité majeure de ce projet est la modification hardware du servomoteur.
*   **Problème** : Normalement, on ordonne un angle au servo, mais on ne sait pas s'il l'a physiquement atteint.
*   **Solution** : Nous avons ouvert le servomoteur et soudé un fil directement sur le curseur de son potentiomètre interne.
*   **Fonctionnement** : Ce fil est relié à l'entrée analogique **A2** (`pin_ANGLE_effectif`). En lisant la tension (0-3.3V/5V), nous calculons l'angle mécanique réel via une formule de conversion. Cela rend la victoire dépendante de la position *physique* réelle et non de la commande logicielle.

### 2. Programmation Non-Bloquante
Le code respecte une contrainte stricte : **Aucune utilisation de `delay()`** dans la boucle de jeu principale (`loop`).
*   L'utilisation de `delay()` bloquerait le processeur, empêchant la lecture du joystick et rendant le contrôle saccadé.
*   À la place, nous utilisons `millis()` pour gérer les temporisations (clignotements, temps de validation de la victoire) tout en continuant à lire les capteurs en permanence.

### 3. Logique de Contrôle
*   **Machine à états implicite** : Le système gère les transitions entre les états (Jeu en cours, Angle trouvé, Victoire, Initialisation) en fonction de l'erreur calculée (`abs(angle_random - angle)`).
*   **Gestion PWM des couleurs** : Les couleurs intermédiaires (comme l'Orange) sont générées en utilisant le PWM (`analogWrite`) pour mélanger le Rouge et le Vert.

### 4. Gestion de l'Affichage
Initialement, l'idée était d'utiliser le **moniteur série** de l'IDE Arduino pour afficher les informations de débogage et de jeu, telles que le chronomètre et l'erreur angulaire (différence entre l'angle cible et l'angle effectif).

Cependant, pour offrir une expérience plus immersive et autonome, nous avons opté pour un **écran OLED SSD1306 (128x64 pixels)** connecté via I2C. Cet écran permet d'afficher en temps réel :
*   Le **timer** (temps écoulé, ou compte à rebours avant la victoire).
*   L'**erreur angulaire** (delta entre l'angle cible et l'angle réel du servomoteur).
Cette approche offre un feedback visuel direct et intégré au projet.

## 📸 Prototype Final

*(Photo du montage complet à venir ici)*