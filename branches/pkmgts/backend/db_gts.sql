/*
MySQL Data Transfer
Source Host: localhost
Source Database: _gts
Target Host: localhost
Target Database: _gts
Date: 20.04.2010 19:22:02
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for events
-- ----------------------------
DROP TABLE IF EXISTS `events`;
CREATE TABLE `events` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text,
  `pkmid` int(11) NOT NULL,
  `pkmname` text NOT NULL,
  `pkmot` text,
  `pkmlvl` text,
  `pkmitem` text,
  `filepkm` text,
  `filewc` text,
  `avfrom` text,
  `avto` text,
  `extra` text,
  `active` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=119 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for tmpvotes
-- ----------------------------
DROP TABLE IF EXISTS `tmpvotes`;
CREATE TABLE `tmpvotes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `ip` text,
  `eventid` int(11) DEFAULT NULL,
  `time` text,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for vars
-- ----------------------------
DROP TABLE IF EXISTS `vars`;
CREATE TABLE `vars` (
  `dummy` int(11) NOT NULL AUTO_INCREMENT,
  `k` text,
  `v` text,
  PRIMARY KEY (`dummy`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for winnerlog
-- ----------------------------
DROP TABLE IF EXISTS `winnerlog`;
CREATE TABLE `winnerlog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `eventid` int(11) DEFAULT NULL,
  `score` text,
  `time` text,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records 
-- ----------------------------
INSERT INTO `events` VALUES ('1', 'Jirachi', '385', 'Jirachi', 'GAMESTP', '5', 'Liechi Berry', '[GAMESTP] Jirachi.pkm', null, '27.02.2010', '13.03.2010', 'Unlocks Night Sky\'s Edge Pokewalker course.', '0');
INSERT INTO `events` VALUES ('2', 'Pichu', '172', 'Pichu', 'GAMESTP', '30', 'Everstone', '[GAMESTP] Pichu.pkm', null, '30.01.2010', '14.02.2010', 'Pikachu Colored Pichu. Unlocks Spiky Eared Pichu in HG/SS.', '0');
INSERT INTO `events` VALUES ('3', 'Arceus', '493', 'Arceus', 'TRU', '100', 'Rowap Berry', '[TRU] Arceus.pkm', null, '07.11.2009', '15.11.2009', 'Unlocks Dialga, Palkia, or Giratina in HG/SS.', '1');
INSERT INTO `events` VALUES ('4', 'Weavile', '461', 'Weavile', 'WORLD09', '30', 'Focus Sash', '[WORLD09] Weavile.pkm', null, null, '15.08.2009', 'World Championship 2009', '1');
INSERT INTO `events` VALUES ('5', 'Milotic', '350', 'Milotic', 'VGC09', '50', 'Flame Orb', '[VGC09] Milotic.pkm', null, '09.05.2009', '13.06.2009', 'Video Game Championships 2009', '1');
INSERT INTO `events` VALUES ('6', 'Regigigas', '486', 'Regigigas', 'TRU', '100', 'Custap Berry', '[TRU] Regigigas.pkm', null, '08.03.2009', '21.03.2009', 'Unlocks Regirock, Regice, and Registeel in Platinum.', '0');
INSERT INTO `events` VALUES ('7', 'Shaymin', '492', 'Shaymin', 'TRU', '50', 'Micle Berry', '[TRU] Shaymin.pkm', null, '08.02.2009', '14.02.2009', 'Unlocks Gracidea Flower in Platinum & HG/SS.', '1');
INSERT INTO `events` VALUES ('8', 'Pikachu', '25', 'Pikachu', 'Nzone', '20', 'Light Ball', '[NZONE] Pikachu.pkm', null, '14.11.2008', '27.11.2008', 'Nintendo Zone', '1');
INSERT INTO `events` VALUES ('9', 'Dragonite', '149', 'Dragonite', 'TRU', '50', 'Yache Berry', '[TRU] Dragonite.pkm', null, '08.11.2008', '09.11.2008', 'Toys R\' Us', '1');
INSERT INTO `events` VALUES ('10', 'Darkrai', '491', 'Darkrai', 'ALAMOS', '50', 'Enigma Berry', '[ALAMOS] Darkrai.pkm', null, '30.10.2008', '09.11.2008', 'Toys R\' Us', '0');
INSERT INTO `events` VALUES ('11', 'Lucario', '448', 'Lucario', 'WORLD08', '30', 'Leftovers', '[WORLD08] Lucario.pkm', null, null, '17.08.2008', 'World Championship 2008', '0');
INSERT INTO `events` VALUES ('12', 'Deoxys', '386', 'Deoxys', 'Gamestp', '50', 'NeverMeltIce', '[GAMESTP] Deoxys.pkm', null, '20.06.2008', '29.06.2008', 'Gamestop', '0');
INSERT INTO `events` VALUES ('13', 'Manaphy', '490', 'Manaphy', 'TRU', '50', 'Red Scarf', '[TRU] Manaphy.pkm', null, null, '29.09.2007', 'Toys R\' Us', '0');
INSERT INTO `events` VALUES ('14', 'Pikachu', '25', 'Pikachu', 'TCGWC', '50', 'Light Ball', '[TCGWC] Pikachu.pkm', null, '08.08.2007', '13.08.2007', 'TCG World Championships 2007', '1');
INSERT INTO `events` VALUES ('15', 'Mew', '151', 'Mew', 'MYSTRY', '10', null, '[MYSTRY] Mew.pkm', null, null, '30.09.2006', 'Toys R\' Us', '0');
INSERT INTO `events` VALUES ('16', 'Arceus', '493', 'Arceus', 'MICHINA', null, null, 'Arceus-02010-MICHINA-ENG PKMDB.com.pkm', null, null, null, 'Netherlands/UK.', '1');
INSERT INTO `events` VALUES ('17', 'Pichu', '172', 'Pichu', 'SPR2010', null, null, 'Pichu-S-03050-SPR2010-ENG PKMDB.com.pkm', null, null, null, null, '1');
INSERT INTO `events` VALUES ('18', 'Jirachi', '385', 'Jirachi', 'PKLATAM', null, null, 'Jirachi-03010-PKLATAM-ENG PKMDB.com.pkm', null, null, null, null, '1');
INSERT INTO `events` VALUES ('19', 'Jirachi', '385', 'Jirachi', 'GAMESTP', null, null, 'Jirachi-02270-GAMESTP-ENG PKMDB.com.pkm', null, null, null, null, '1');
INSERT INTO `events` VALUES ('20', 'Pichu', '172', 'Pichu', 'GAMESTP', null, null, 'Pichu-S-01300-GAMESTP-ENG PKMDB.com.pkm', null, null, null, null, '1');
INSERT INTO `events` VALUES ('21', 'Shaymin', '492', 'Shaymin', 'Movie11', null, null, 'Shaymin-04019-Movie11-ENG PKMDB.com.pkm', null, null, null, null, '1');
INSERT INTO `events` VALUES ('22', 'Vlad\'s Arceus', '493', 'Arceus', 'Vlad', '100', 'Electric Plate', '[Vlad] Arceus.pkm', null, null, null, 'Custom made Arceus -based on the one from Toys R\' Us.', '1');
