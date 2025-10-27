#include "gbc.h"

// clang-format off
static const char *old_lic[]={[0x00]="None",[0x01]="Nintendo",[0x08]=
    "Capcom",[0x09]="HOT-B",[0x0A]="Jaleco",[0x0B]="CoconutsJapan",
    [0x0C]="EliteSystems",[0x13]="EA(ElectronicArts)",[0x18]=
        "HudsonSoft",[0x19]="ITCEntertainment",[0x1A]="Yanoman",[0x1D]=
            "JapanClary",[0x1F]="VirginGamesLtd.3",[0x24]="PCMComplete",
    [0x25]="San-X",[0x28]="Kemco",[0x29]="SETACorporation",[0x30]=
        "Infogrames5",[0x31]="Nintendo",[0x32]="Bandai",[0x33]=
            "NEWLICSHOULDBEUSEDINSTEAD!!!!",[0x34]="Konami",[0x35]=
            "HectorSoft",[0x38]="Capcom",[0x39]="Banpresto",[0x3C]=
            "EntertainmentInteractive(stub)",[0x3E]="Gremlin",[0x41]=
            "UbiSoft1",[0x42]="Atlus",[0x44]="MalibuInteractive",[0x46]
            ="Angel",[0x47]="SpectrumHoloByte",[0x49]="Irem",[0x4A]=
            "VirginGamesLtd.3",[0x4D]="MalibuInteractive",[0x4F]=
            "U.S.Gold",[0x50]="Absolute",[0x51]="AcclaimEntertainment",
    [0x52]="Activision",[0x53]="SammyUSACorporation",[0x54]="GameTek",
    [0x55]="ParkPlace15",[0x56]="LJN",[0x57]="Matchbox",[0x59]=
        "MiltonBradleyCompany",[0x5A]="Mindscape",[0x5B]="Romstar",
    [0x5C]="NaxatSoft16",[0x5D]="Tradewest",[0x60]="TitusInteractive",
    [0x61]="VirginGamesLtd.3",[0x67]="OceanSoftware",[0x69]=
        "EA(ElectronicArts)",[0x6E]="EliteSystems",[0x6F]="ElectroBrain",
    [0x70]="Infogrames5",[0x71]="InterplayEntertainment",[0x72]=
        "Broderbund",[0x73]="SculpturedSoftware6",[0x75]=
            "TheSalesCurveLimited7",[0x78]="THQ",[0x79]="Accolade8",
    [0x7A]="TriffixEntertainment",[0x7C]="MicroProse",[0x7F]="Kemco",
    [0x80]="MisawaEntertainment",[0x83]="LOZCG.",[0x86]="TokumaShoten",
    [0x8B]="Bullet-ProofSoftware2",[0x8C]="VicTokaiCorp.17",[0x8E]=
        "ApeInc.18",[0x8F]="I’Max19",[0x91]="ChunsoftCo.9",[0x92]=
            "VideoSystem",[0x93]="TsubarayaProductions",[0x95]="Varie",
    [0x96]="Yonezawa10/S’Pal",[0x97]="Kemco",[0x99]="Arc",[0x9A]=
        "NihonBussan",[0x9B]="Tecmo",[0x9C]="Imagineer",[0x9D]=
            "Banpresto",[0x9F]="Nova",[0xA1]="HoriElectric",[0xA2]=
            "Bandai",[0xA4]="Konami",[0xA6]="Kawada",[0xA7]="Takara",
    [0xA9]="TechnosJapan",[0xAA]="Broderbund",[0xAC]="ToeiAnimation",
    [0xAD]="Toho",[0xAF]="Namco",[0xB0]="AcclaimEntertainment",[0xB1]
        ="ASCIICorporationorNexsoft",[0xB2]="Bandai",[0xB4]=
            "SquareEnix",[0xB6]="HALLaboratory",[0xB7]="SNK",[0xB9]=
            "PonyCanyon",[0xBA]="CultureBrain",[0xBB]="Sunsoft",[0xBD]=
            "SonyImagesoft",[0xBF]="SammyCorporation",[0xC0]="Taito",
    [0xC2]="Kemco",[0xC3]="Square",[0xC4]="TokumaShoten",[0xC5]=
        "DataEast",[0xC6]="TonkinHouse",[0xC8]="Koei",[0xC9]="UFL",
    [0xCA]="UltraGames",[0xCB]="VAP,Inc.",[0xCC]="UseCorporation",
    [0xCD]="Meldac",[0xCE]="PonyCanyon",[0xCF]="Angel",[0xD0]=
        "Taito",[0xD1]="SOFEL(SoftwareEngineeringLab)",[0xD2]="Quest",
    [0xD3]="SigmaEnterprises",[0xD4]="ASKKodanshaCo.",[0xD6]=
        "NaxatSoft16",[0xD7]="CopyaSystem",[0xD9]="Banpresto",[0xDA]=
            "Tomy",[0xDB]="LJN",[0xDD]="NipponComputerSystems",[0xDE]=
            "HumanEnt.",[0xDF]="Altron",[0xE0]="Jaleco",[0xE1]=
            "TowaChiki",[0xE2]="Yutaka#Needsmoreinfo",[0xE3]="Varie",
    [0xE5]="Epoch",[0xE7]="Athena",[0xE8]="AsmikAceEntertainment",
    [0xE9]="Natsume",[0xEA]="KingRecords",[0xEB]="Atlus",[0xEC]=
        "Epic/SonyRecords",[0xEE]="IGS",[0xF0]="AWave",[0xF3]=
            "ExtremeEntertainment",[0xFF]="LJN",};
// clang-format on

// TODO: define NEW licensee names

const char *gbc_lic(gb_chdr_t *chdr) { return old_lic[chdr->old_licensee]; }
