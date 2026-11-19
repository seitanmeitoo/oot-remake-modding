// The items, songs and quests of Cinema's Inventory and Story tabs.
//
// Generated from Slopped Up Sloperina's src/data.c (sloperinaGZ 0.0.4.2 by PracticeROM, GPL-3.0-or-later),
// its item, gear, mask, trade, ammo, quest item, song and quest tables, by a one-off script.
// Changed: the labels are translated into French; the quests' stage names stay the game's.
#include "gamedata.h"
#include "inventory.h"

const inv_item_t inv_items[] = {
  { "Bâtons Mojo", IK_ACQUIRED, { 0, FT_NONE }, 0xc4e21ac2, NULL, 0, NULL },
  { "Lance-Pierre", IK_OWNED, { 0x80d34ce9, FT_INT }, 0x0ec22ee1, "SlingShot", 0, NULL },
  { "Boomerang", IK_OWNED, { 0xa1d76d31, FT_INT }, 0x7f4ce38d, "Boomerang", 0, NULL },
  { "Noix Mojo", IK_ACQUIRED, { 0, FT_NONE }, 0x00c2ee37, NULL, 0, NULL },
  { "Bombes", IK_ACQUIRED, { 0, FT_NONE }, 0x2bb00768, NULL, 0, NULL },
  { "Missiles", IK_ACQUIRED, { 0, FT_NONE }, 0x5ab49c8e, NULL, 0, NULL },
  { "Masse des Titans", IK_OWNED, { 0x1aafdb66, FT_INT }, 0x7227efd8, "Hammer", 0, NULL },
  { "Grappin", IK_OWNED, { 0x8573d2cd, FT_INT }, 0xff98ed58, "Hookshot", 0, NULL },
  { "Super Grappin", IK_OWNED, { 0x8499ea34, FT_INT }, 0x38aea3bb, "Longshot", 0, NULL },
  { "Arc des Fées", IK_OWNED, { 0x938d56bf, FT_INT }, 0x69ec28be, "Bow", 0, NULL },
  { "Flèche de Feu", IK_OWNED, { 0x5e529ab9, FT_INT }, 0x4db7eae0, "FireArrow", 0, NULL },
  { "Flèche de Glace", IK_OWNED, { 0x40735a8d, FT_INT }, 0x5c8f3099, "IceArrow", 0, NULL },
  { "Flèche de Lumière", IK_OWNED, { 0xbfb285c8, FT_INT }, 0x8637bab4, "LightArrow", 0, NULL },
  { "Haricot magique", IK_ACQUIRED, { 0, FT_NONE }, 0x6d39c600, NULL, 0, NULL },
  { "Feu de Din", IK_OWNED, { 0xb796c16d, FT_INT }, 0xfa04e611, "DinFire", 0, NULL },
  { "Vent de Farore", IK_OWNED, { 0x47f2ca87, FT_INT }, 0xffabf058, "FaroreWind", 0, NULL },
  { "Amour de Nayru", IK_OWNED, { 0xf0828cd0, FT_INT }, 0x00b44241, "NayruLove", 0, NULL },
  { "Échange (enfant)", IK_TRADE_CHILD, { 0, FT_NONE }, 0, NULL, 0, NULL },
  { "Échange (adulte)", IK_TRADE_ADULT, { 0, FT_NONE }, 0, NULL, 0, NULL },
  { "Monocle de Vérité", IK_OWNED, { 0x49d1c374, FT_INT }, 0x7fe2a5b2, "GlassTruth", 0, NULL },
  { "Carte Gerudo", IK_OWNED, { 0xfeb4d56b, FT_INT }, 0x0e6264c7, NULL, 0, NULL },
  { "Ocarina", IK_OCARINA, { 0, FT_NONE }, 0, NULL, 0, NULL },
};
const int inv_n_items = sizeof(inv_items) / sizeof(*inv_items);

const inv_item_t inv_gear[] = {
  { "Épée Kokiri", IK_OWNED, { 0xeeb495b9, FT_INT }, 0x405d8a0f, NULL, GD_STR_SWORD, "Weapon_Sword_Kokiri" },
  { "Épée de Légende", IK_OWNED, { 0xf1c45294, FT_INT }, 0x7ae38e5b, NULL, GD_STR_SWORD, "Weapon_Sword_Master" },
  { "Lame des Géants", IK_OWNED, { 0x396a5c02, FT_INT }, 0x1e824efa, NULL, GD_STR_SWORD, "Weapon_Sword_Giant" },
  { "Lame brisée", IK_OWNED, { 0x8d02f5ea, FT_INT }, 0x720df203, NULL, GD_STR_SWORD, "Weapon_Sword_BrokenGiant" },
  { "Épée de Biggoron", IK_OWNED, { 0xb0b3e399, FT_INT }, 0x33466812, NULL, GD_STR_SWORD, "Weapon_Sword_Biggoron" },
  { "Bouclier Mojo", IK_OWNED, { 0x7ca64f23, FT_INT }, 0x5217da1e, NULL, GD_STR_SHIELD, "Weapon_Shield_Deku" },
  { "Bouclier hylien", IK_OWNED, { 0x08befd3a, FT_INT }, 0x2f817d54, NULL, GD_STR_SHIELD, "Weapon_Shield_Hylian" },
  { "Bouclier miroir", IK_OWNED, { 0xf58e0e2f, FT_INT }, 0x75e94e42, NULL, GD_STR_SHIELD, "Weapon_Shield_Mirror" },
  { "Tunique Goron", IK_OWNED, { 0x21ecbd19, FT_INT }, 0xae5611af, NULL, GD_STR_TUNIC, "Armor_Adult_Goron" },
  { "Tunique Zora", IK_OWNED, { 0xb816af94, FT_INT }, 0x4f5a929c, NULL, GD_STR_TUNIC, "Armor_Adult_Zora" },
  { "Bottes de plomb", IK_OWNED, { 0x2bad189a, FT_INT }, 0x5fa190e6, "HeavyBoot", GD_STR_BOOTS, "Armor_Adult_HeavyBoot" },
  { "Bottes des airs", IK_OWNED, { 0xc9220366, FT_INT }, 0x1109c920, "HoverBoot", GD_STR_BOOTS, "Armor_Adult_HoverBoot" },
  { "Ocarina des Fées", IK_OWNED, { 0xb7df60f1, FT_INT }, 0x42bd296f, NULL, 0, NULL },
  { "Ocarina du Temps", IK_OWNED, { 0x54e2a3d1, FT_INT }, 0x1da2c511, NULL, 0, NULL },
};
const int inv_n_gear = sizeof(inv_gear) / sizeof(*inv_gear);

const pouch_one_t inv_masks[] = {
  { "Masque du Renard", "Armor_Child_Mask_Keaton", "KeatonMask" },
  { "Masque de Mort", "Armor_Child_Mask_Skull", "SkullMask" },
  { "Masque d'Effroi", "Armor_Child_Mask_Spooky", "SpookyMask" },
  { "Masque du Lapin", "Armor_Child_Mask_Bunny", "BunnyMask" },
  { "Masque Goron", "Armor_Child_Mask_Goron", "GoronMask" },
  { "Masque Zora", "Armor_Child_Mask_Zora", "ZoraMask" },
  { "Masque Gerudo", "Armor_Child_Mask_Gerudo", "GerudoMask" },
  { "Masque de Vérité", "Armor_Child_Mask_Truth", "TruthMask" },
};
const int inv_n_masks = sizeof(inv_masks) / sizeof(*inv_masks);

const pouch_one_t inv_trade_child[] = {
  { "Oeuf bizarre", "Item_WeirdEgg", "WeirdEgg" },
  { "Poulette", "Item_AlarmCucco", "AlarmCucco" },
  { "Lettre de Zelda", "Item_ZeldaLetter", "ZeldaLetter" },
};
const int inv_n_trade_child = sizeof(inv_trade_child) / sizeof(*inv_trade_child);

const pouch_one_t inv_trade_adult[] = {
  { "Oeuf de poche", "Item_PocketEgg", "PocketEgg" },
  { "Poulette de poche", "Item_PocketCucco", "PocketCucco" },
  { "Cojiro", "Item_CojiroCucco", "CojiroCucco" },
  { "Champignon suspect", "Item_OddMushroom", "OddMushroom" },
  { "Mixture suspecte", "Item_OddPowder", "OddPowder" },
  { "Scie", "Item_PoacherSaw", "PoacherSaw" },
  { "Épée brisée", "Item_BrokenSword", "BrokenSword" },
  { "Ordonnance", "Item_Prescription", "Prescription" },
  { "Crapaud-Oeil", "Item_EyeballFrog", "EyeballFrog" },
  { "Super Gouttes", "Item_EyeDrop", "EyeDrop" },
  { "Certificat", "Item_ClaimCheck", "ClaimCheck" },
};
const int inv_n_trade_adult = sizeof(inv_trade_adult) / sizeof(*inv_trade_adult);

const ammo_t inv_ammo[] = {
  { "Bâtons Mojo", 0xb9494411, 0x457616b0, 0, 0xc4e21ac2 },
  { "Noix Mojo", 0xf30d9f54, 0x735b00f7, 0, 0x00c2ee37 },
  { "Graines Mojo", 0x5fadf7fc, 0xae8cc742, 0, 0x4d779571 },
  { "Bombes", 0xdb1bcd4f, 0xe753508b, 0, 0x2bb00768 },
  { "Flèches", 0x518ec74b, 0x418db791, 0, 0x572701ea },
  { "Missiles", 0xdf7c517a, 0x00000000, 50, 0x5ab49c8e },
  { "Haricots magiques", 0xb003124d, 0x00000000, 10, 0x6d39c600 },
};
const int inv_n_ammo = sizeof(inv_ammo) / sizeof(*inv_ammo);

const flag_t inv_quest_items[] = {
  { "Émeraude Kokiri", { { 0x110b83ec, FT_BOOL }, { 0xcbd1e58c, FT_INT } }, 0 },
  { "Rubis Goron", { { 0x8908d6ad, FT_BOOL }, { 0x7d69ad47, FT_INT } }, 0 },
  { "Saphir Zora", { { 0xeec383f5, FT_BOOL }, { 0xb8e3bf45, FT_INT } }, 0 },
  { "Médaillon Forêt", { { 0xfcd8d653, FT_BOOL }, { 0x692d57cb, FT_INT } }, 0 },
  { "Médaillon Feu", { { 0x103a0dc7, FT_BOOL }, { 0x5ce254a6, FT_INT } }, 0 },
  { "Médaillon Eau", { { 0xcf64e780, FT_BOOL }, { 0x0151a68f, FT_INT } }, 0 },
  { "Médaillon Esprit", { { 0xe269df38, FT_BOOL }, { 0x0c7a5fd0, FT_INT } }, 0 },
  { "Médaillon Ombre", { { 0x12d63ac6, FT_BOOL }, { 0x204d2063, FT_INT } }, 0 },
  { "Médaillon Lumière", { { 0x9026c187, FT_BOOL }, { 0x235c7d9a, FT_INT } }, 0 },
};
const int inv_n_quest_items = sizeof(inv_quest_items) / sizeof(*inv_quest_items);

const flag_t inv_songs[] = {
  { "Berceuse de Zelda", { { 0x01c20002, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Chant d'Epona", { { 0xb492ac33, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Chant de Saria", { { 0xd3f8bfcc, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Chant du Soleil", { { 0x165dd1ad, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Chant du Temps", { { 0x03b3450e, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Chant des Tempêtes", { { 0x2b39dd4f, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Chant Épouvantail", { { 0x5e86fb4d, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Menuet des Bois", { { 0xdc0a0b54, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Boléro du Feu", { { 0x49041ab9, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Sérénade de l'Eau", { { 0x3fa63986, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Requiem de l'Esprit", { { 0x802a36a8, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Nocturne de l'Ombre", { { 0xc2fb95f4, FT_BOOL }, { 0, FT_NONE } }, 0 },
  { "Prélude de la Lumière", { { 0xd6320ba4, FT_BOOL }, { 0, FT_NONE } }, 0 },
};
const int inv_n_songs = sizeof(inv_songs) / sizeof(*inv_songs);

static const char *const stages_0[] = { "NotReady", "Ready", "Step01", "Step01_00", "Step01_00_01", "Step01_00_02", "Step01_00_03", "Step01_01", "Step01_02", "Step02", "Complete" };
static const char *const stages_1[] = { "NotReady", "Ready", "StartSubEvent", "WaitEggHatch", "EggHatched", "Step01", "Step01_01", "Step02", "Step03", "Step03_01", "Step03_02", "Step03_03", "Step04", "Step05", "Step05_01", "Step06", "Step06_01", "Step06_02", "Step06_03", "Step07", "Step08", "Step08_01", "Step09", "Step09_01", "Step10", "Step11", "Complete" };
static const char *const stages_2[] = { "NotReady", "Ready", "Retry", "Playing", "Finish", "Complete" };
static const char *const stages_3[] = { "NotReady", "Ready", "Retry", "Playing", "Finish", "Complete" };
static const char *const stages_4[] = { "NotReady", "Ready", "Reset", "Step01", "Step02", "Step03", "Step04", "Step05", "Step06", "Step07", "Step08", "Step09", "Step10", "Step11", "Step12", "Step13", "Complete" };
static const char *const stages_5[] = { "NotReady", "Ready", "Playing", "PlayingSecond", "Finish", "FinishAfterTalk", "Dropout", "Courseout", "RetryCheck", "Complete" };
static const char *const stages_6[] = { "NotReady", "Ready", "Step01", "Step02", "Step03", "Step04", "Step05", "Step06", "Step07", "Step08", "Step09", "Complete" };
static const char *const stages_7[] = { "NotReady", "Ready", "GoInside", "MeetBoss", "GoToBossRoom", "DefeatBoss", "GoToWarpPoint", "Complete" };
static const char *const stages_8[] = { "NotReady", "Ready", "Step01", "Step02", "Step03", "Complete" };
static const char *const stages_9[] = { "NotReady", "Ready", "KakarikoVillageNpc", "KakarikoVillageLetter", "DeathMtTrailIn", "DeathMtTrailNpc", "GoronCityIn", "GoronCityDarniaroomNpcTalk01", "GoronCityDarniaroomOpen", "GoronCityDarniaTalk", "DarniaRoomKokiriTalk", "KokiriForestNPCTalk", "KokiriForestFire", "KokiriForestIn", "SariaMelodyGet", "SariaMelodyPlay", "DodongoCavernOpne", "DungeonStart", "BossBattle", "EscapeDungeon", "Complete" };
static const char *const stages_10[] = { "NotReady", "Ready", "Step01", "Step02", "Step2_1", "Srep2_1_1", "Step2_2", "Step03", "Step04", "Step05", "Step06", "Complete" };
static const char *const stages_11[] = { "NotReady", "Ready", "GuideToLonLonRanch", "GuideToLonLonRanchAdult", "ReadyForGetEponaSong", "AfterGetEponaSong", "ReadyForGambleRace", "AfterTalkMalon", "GambleRacePlaying", "AfterWonGambleRace", "Complete" };
static const char *const stages_12[] = { "NotReady", "Ready", "Retry", "Playing", "GetReady", "Finish", "Complete" };
static const char *const stages_13[] = { "NotReady", "Ready", "Step01", "Complete" };
static const char *const stages_14[] = { "NotReady", "Ready", "StopDaruniaJr", "TalkDaruniaJr", "MoveStatue", "EnterCrater", "MeetSheik", "EnterTemple", "MeetDarunia", "GetBossKey", "MeetBoss", "DefeatBoss", "GoToWarpPoint", "Complete" };
static const char *const stages_15[] = { "NotReady", "Ready", "Step01", "Complete" };
static const char *const stages_16[] = { "NotReady", "Ready", "Step01", "Step02", "Step03", "Step04", "Step05", "Step06", "Step06_1_0", "Step06_1_1", "Step06_1_2", "Step06_2", "Step07", "Step08", "Step09", "Complete" };
static const char *const stages_17[] = { "NotReady", "Ready", "Step01", "Step02", "Step03", "Complete" };
static const char *const stages_18[] = { "NotReady", "Ready", "Step00", "Step01", "Complete" };
static const char *const stages_19[] = { "NotReady", "Ready", "MeetPierre", "MeetBonooru", "IntoThePast", "PlayScarecrowSong", "IntoTheFuture", "Complete" };
static const char *const stages_20[] = { "NotReady", "Ready", "ReadyForGetStormSong", "step02", "step03", "BeforeBossRoom", "step04", "step05", "Complete" };
static const char *const stages_21[] = { "NotReady", "Ready", "Playing", "Complete" };
static const char *const stages_22[] = { "NotReady", "Ready", "Retry", "Playing", "Finish", "Complete" };
static const char *const stages_23[] = { "NotReady", "Ready", "Playing", "Complete" };
static const char *const stages_24[] = { "NotReady", "Ready", "Step01", "Step02", "Step03", "Step03_Impa", "Step04", "Step05", "Step05_01", "Step06", "Step07", "Step08", "Step09", "Step10", "Step11", "Step12", "Step13", "Step13_01", "Step14", "Complete" };
static const char *const stages_25[] = { "NotReady", "Ready", "Step01", "Step02", "Step03", "Complete" };
static const char *const stages_26[] = { "NotReady", "Ready", "UntilRentMaskOfKeaton", "Step01_Hint", "Step01", "Step02", "UntilRentMaskOfSkull", "Step03_Hint", "Step03", "Step04", "UntilRentMaskOfSpooky", "Step05_Hint", "Step05", "Step06", "UntilRentMaskOfBunny", "Step07_Hint", "Step07", "Step08", "Complete" };
static const char *const stages_27[] = { "NotReady", "Ready", "GoToTopOfMountain", "GoToFountain", "GetFairyBreath", "GoToHyruleCastle", "Complete" };
static const char *const stages_28[] = { "NotReady", "Ready", "Playing", "Complete" };
static const char *const stages_29[] = { "NotReady", "Ready", "GoToDungeon", "GotGlassTruth", "GetGlassTruth", "MeetBoss", "BattleOnShip", "BeforeBossRoom", "DefeatBoss", "GoToWarpPoint", "Complete" };
static const char *const stages_30[] = { "NotReady", "Ready", "Playing", "Complete" };
static const char *const stages_31[] = { "NotReady", "Ready", "CollectToken", "Finish", "Complete" };
static const char *const stages_32[] = { "NotReady", "Ready", "Step01", "Step02", "Step03", "Step04", "Step05", "Step05_01", "Step05_02", "Step05_03", "Step06", "Step07", "Step07_pre00", "Step07_00", "Step08", "Step08_01", "Step09", "Step09_01", "Step10", "Complete" };
static const char *const stages_33[] = { "NotReady", "Ready", "Step00", "Step01", "Step03", "Step04", "Step05", "Step06", "Step07", "Step07_00", "Step07_1", "Step07_1_Safety", "Step07_2", "Step08", "Complete" };
static const char *const stages_34[] = { "NotReady", "Ready", "Step01", "Step02", "Step02_01", "Step03", "Step03_01", "Step03_02", "Step04", "Complete" };
const quest_t inv_quests[] = {
  { "Tous les Sages", 0x003694b5, 11, stages_0 },
  { "Épée de Biggoron", 0x26a2e01c, 27, stages_1 },
  { "Bowling Teigneux", 0x7a6d0096, 6, stages_2 },
  { "Boss Rush", 0xbe19d91a, 6, stages_3 },
  { "Fuite du château", 0x4661f050, 17, stages_4 },
  { "Course d'Igor", 0xe73daf62, 10, stages_5 },
  { "Vaincre Ganon", 0x746f47c0, 12, stages_6 },
  { "Arbre Mojo", 0x7d776b61, 8, stages_7 },
  { "Plongée", 0xa4ed3902, 6, stages_8 },
  { "Caverne Dodongo", 0x1002b466, 21, stages_9 },
  { "Porte du Temps", 0x378913ca, 12, stages_10 },
  { "Epona", 0x92431c55, 11, stages_11 },
  { "Retrouver les poules", 0x3e7d10c0, 7, stages_12 },
  { "Retrouver Richard", 0x93e84ee3, 4, stages_13 },
  { "Temple du Feu", 0xeea87aec, 14, stages_14 },
  { "Pêche", 0xf6e99816, 4, stages_15 },
  { "Temple de la Forêt", 0xa9632cca, 16, stages_16 },
  { "Flèche de Feu", 0x110f26e4, 6, stages_17 },
  { "Flèche de Glace", 0xf7176a52, 5, stages_18 },
  { "Épouvantail", 0x9472302b, 8, stages_19 },
  { "Chant des Tempêtes", 0x7a5b9537, 9, stages_20 },
  { "Boutique des Esprits", 0x6e9b2539, 4, stages_21 },
  { "Tir sur cible", 0xa3404532, 6, stages_22 },
  { "Course de chevaux", 0xd601ee20, 4, stages_23 },
  { "Jabu-Jabu", 0x5bca02a6, 20, stages_24 },
  { "Épée Kokiri", 0x3372689b, 6, stages_25 },
  { "Vendeur de masques", 0xea010310, 19, stages_26 },
  { "Grande Fée", 0x682d1169, 7, stages_27 },
  { "Course à pied", 0xf6eb33a1, 4, stages_28 },
  { "Temple de l'Ombre", 0x7a91dbd5, 11, stages_29 },
  { "Chant de Skull Kid", 0x474f4ca5, 4, stages_30 },
  { "Skulltulas", 0x8c5e9f84, 5, stages_31 },
  { "Temple de l'Esprit", 0x19189e9d, 20, stages_32 },
  { "Temple de l'Eau", 0x560a64af, 15, stages_33 },
  { "Zelda et le château", 0xdb89c1a9, 10, stages_34 },
};
const int inv_n_quests = sizeof(inv_quests) / sizeof(*inv_quests);
