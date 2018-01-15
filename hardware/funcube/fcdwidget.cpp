//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
// Copyright 2014 F4GKR Sylvain AZARIAN . All rights reserved.
// SPECIAL THANKS TO F6BHI FOR LETTING ME TESTING THE FUNCUBE
//
//Redistribution and use in source and binary forms, with or without modification, are
//permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice, this list of
//	  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice, this list
//	  of conditions and the following disclaimer in the documentation and/or other materials
//	  provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY Sylvain AZARIAN F4GKR ``AS IS'' AND ANY EXPRESS OR IMPLIED
//WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Sylvain AZARIAN OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//The views and conclusions contained in the software and documentation are those of the
//authors and should not be interpreted as representing official policies, either expressed
//or implied, of Sylvain AZARIAN F4GKR.
//==========================================================================================
/***************************************************************************
 *  This file contains blocks of code from Qthid.
 *
 *  Copyright (C) 2010  Howard Long, G6LVB
 *  CopyRight (C) 2011  Alexandru Csete, OZ9AEC
 *                      Mario Lorenz, DL5MLO
 *
 *  Qthid is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Qthid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Qthid.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/
#include "funcube.h"
#include "fcdwidget.h"
#include <QDebug>

#include "fcdhidcmd.h"


/** \brief LNA gain options */
static const COMBO_ITEM_STRUCT _cisLNAGain[]=
{
    {QT_TR_NOOP("-5.0dB"),TLGE_N5_0DB},
    {QT_TR_NOOP("-2.5dB"),TLGE_N2_5DB},
    {QT_TR_NOOP("+0.0dB"),TLGE_P0_0DB},
    {QT_TR_NOOP("+2.5dB"),TLGE_P2_5DB},
    {QT_TR_NOOP("+5,0dB"),TLGE_P5_0DB},
    {QT_TR_NOOP("+7.5dB"),TLGE_P7_5DB},
    {QT_TR_NOOP("+10.0dB"),TLGE_P10_0DB},
    {QT_TR_NOOP("+12.5dB"),TLGE_P12_5DB},
    {QT_TR_NOOP("+15.0dB"),TLGE_P15_0DB},
    {QT_TR_NOOP("+17.5dB"),TLGE_P17_5DB},
    {QT_TR_NOOP("+20.0dB"),TLGE_P20_0DB},
    {QT_TR_NOOP("+25.0dB"),TLGE_P25_0DB},
    {QT_TR_NOOP("+30.0dB"),TLGE_P30_0DB},
    {NULL,0}
};

/** \brief LNA enhance options */
static const COMBO_ITEM_STRUCT _cisLNAEnhance[]=
{
    {QT_TR_NOOP("Off"),TLEE_OFF},
    {QT_TR_NOOP("0"),TLEE_0},
    {QT_TR_NOOP("1"),TLEE_1},
    {QT_TR_NOOP("2"),TLEE_2},
    {QT_TR_NOOP("3"),TLEE_3},
    {NULL,0}
};

/** \brief Band selector options */
static const COMBO_ITEM_STRUCT _cisBand[]=
{
    {QT_TR_NOOP("VHF II"),TBE_VHF2},
    {QT_TR_NOOP("VHF III"),TBE_VHF3},
    {QT_TR_NOOP("UHF"),TBE_UHF},
    {QT_TR_NOOP("L band"),TBE_LBAND},
    {NULL,0}
};

/** \brief RF filter band 0 options */
static const COMBO_ITEM_STRUCT _cisRFFilter0[]=
{
    {QT_TR_NOOP("268MHz LPF"),TRFE_LPF268MHZ},
    {QT_TR_NOOP("299MHz LPF"),TRFE_LPF299MHZ},
    {NULL,0}
};

/** \brief RF filter band 1 options */
static const COMBO_ITEM_STRUCT _cisRFFilter1[]=
{
    {QT_TR_NOOP("509MHz LPF"),TRFE_LPF509MHZ},
    {QT_TR_NOOP("656MHz LPF"),TRFE_LPF656MHZ},
    {NULL,0}
};

/** \brief RF filter band 2 options */
static const COMBO_ITEM_STRUCT _cisRFFilter2[]=
{
    {QT_TR_NOOP("360MHz BPF"),TRFE_BPF360MHZ},
    {QT_TR_NOOP("380MHz BPF"),TRFE_BPF380MHZ},
    {QT_TR_NOOP("405MHz BPF"),TRFE_BPF405MHZ},
    {QT_TR_NOOP("425MHz BPF"),TRFE_BPF425MHZ},
    {QT_TR_NOOP("450MHz BPF"),TRFE_BPF450MHZ},
    {QT_TR_NOOP("475MHz BPF"),TRFE_BPF475MHZ},
    {QT_TR_NOOP("505MHz BPF"),TRFE_BPF505MHZ},
    {QT_TR_NOOP("540MHz BPF"),TRFE_BPF540MHZ},
    {QT_TR_NOOP("575MHz BPF"),TRFE_BPF575MHZ},
    {QT_TR_NOOP("615MHz BPF"),TRFE_BPF615MHZ},
    {QT_TR_NOOP("670MHz BPF"),TRFE_BPF670MHZ},
    {QT_TR_NOOP("720MHz BPF"),TRFE_BPF720MHZ},
    {QT_TR_NOOP("760MHz BPF"),TRFE_BPF760MHZ},
    {QT_TR_NOOP("840MHz BPF"),TRFE_BPF840MHZ},
    {QT_TR_NOOP("890MHz BPF"),TRFE_BPF890MHZ},
    {QT_TR_NOOP("970MHz BPF"),TRFE_BPF970MHZ},
    {NULL,0}
};

/** \brief RF filter band 3 options */
static const COMBO_ITEM_STRUCT _cisRFFilter3[]=
{
    {QT_TR_NOOP("1300MHz BPF"),TRFE_BPF1300MHZ},
    {QT_TR_NOOP("1320MHz BPF"),TRFE_BPF1320MHZ},
    {QT_TR_NOOP("1360MHz BPF"),TRFE_BPF1360MHZ},
    {QT_TR_NOOP("1410MHz BPF"),TRFE_BPF1410MHZ},
    {QT_TR_NOOP("1445MHz BPF"),TRFE_BPF1445MHZ},
    {QT_TR_NOOP("1460MHz BPF"),TRFE_BPF1460MHZ},
    {QT_TR_NOOP("1490MHz BPF"),TRFE_BPF1490MHZ},
    {QT_TR_NOOP("1530MHz BPF"),TRFE_BPF1530MHZ},
    {QT_TR_NOOP("1560MHz BPF"),TRFE_BPF1560MHZ},
    {QT_TR_NOOP("1590MHz BPF"),TRFE_BPF1590MHZ},
    {QT_TR_NOOP("1640MHz BPF"),TRFE_BPF1640MHZ},
    {QT_TR_NOOP("1660MHz BPF"),TRFE_BPF1660MHZ},
    {QT_TR_NOOP("1680MHz BPF"),TRFE_BPF1680MHZ},
    {QT_TR_NOOP("1700MHz BPF"),TRFE_BPF1700MHZ},
    {QT_TR_NOOP("1720MHz BPF"),TRFE_BPF1720MHZ},
    {QT_TR_NOOP("1750MHz BPF"),TRFE_BPF1750MHZ},
    {NULL,0}
};

/** \brief Mixer gain options */
static const COMBO_ITEM_STRUCT _cisMixerGain[]=
{
    {QT_TR_NOOP("4dB"),TMGE_P4_0DB},
    {QT_TR_NOOP("12dB"),TMGE_P12_0DB},
    {NULL,0}
};

/** \brief Bias options */
static const COMBO_ITEM_STRUCT _cisBiasCurrent[]=
{
    {QT_TR_NOOP("00 L band"),TBCE_LBAND},
    {QT_TR_NOOP("01"),TBCE_1},
    {QT_TR_NOOP("10"),TBCE_2},
    {QT_TR_NOOP("11 V/U band"),TBCE_VUBAND},
    {NULL,0}
};

/** \brief Mixer filter options */
static const COMBO_ITEM_STRUCT _cisMixerFilter[]=
{
    {QT_TR_NOOP("1.9MHz"),TMFE_1_9MHZ},
    {QT_TR_NOOP("2.3MHz"),TMFE_2_3MHZ},
    {QT_TR_NOOP("2.7MHz"),TMFE_2_7MHZ},
    {QT_TR_NOOP("3.0MHz"),TMFE_3_0MHZ},
    {QT_TR_NOOP("3.4MHz"),TMFE_3_4MHZ},
    {QT_TR_NOOP("3.8MHz"),TMFE_3_8MHZ},
    {QT_TR_NOOP("4.2MHz"),TMFE_4_2MHZ},
    {QT_TR_NOOP("4.6MHz"),TMFE_4_6MHZ},
    {QT_TR_NOOP("27MHz"),TMFE_27_0MHZ},
    {NULL,0}
};

/** \brief IF gain 1 options */
static const COMBO_ITEM_STRUCT _cisIFGain1[]=
{
    {QT_TR_NOOP("-3dB"),TIG1E_N3_0DB},
    {QT_TR_NOOP("+6dB"),TIG1E_P6_0DB},
    {NULL,0}
};

/** \brief IF gain mode options */
static const COMBO_ITEM_STRUCT _cisIFGainMode[]=
{
    {QT_TR_NOOP("Linearity"),TIGME_LINEARITY},
    {QT_TR_NOOP("Sensitivity"),TIGME_SENSITIVITY},
    {NULL,0}
};

/** \brief IF RC filter options */
static const COMBO_ITEM_STRUCT _cisIFRCFilter[]=
{
    {QT_TR_NOOP("1.0MHz"),TIRFE_1_0MHZ},
    {QT_TR_NOOP("1.2MHz"),TIRFE_1_2MHZ},
    {QT_TR_NOOP("1.8MHz"),TIRFE_1_8MHZ},
    {QT_TR_NOOP("2.6MHz"),TIRFE_2_6MHZ},
    {QT_TR_NOOP("3.4MHz"),TIRFE_3_4MHZ},
    {QT_TR_NOOP("4.4MHz"),TIRFE_4_4MHZ},
    {QT_TR_NOOP("5.3MHz"),TIRFE_5_3MHZ},
    {QT_TR_NOOP("6.4MHz"),TIRFE_6_4MHZ},
    {QT_TR_NOOP("7.7MHz"),TIRFE_7_7MHZ},
    {QT_TR_NOOP("9.0MHz"),TIRFE_9_0MHZ},
    {QT_TR_NOOP("10.6MHz"),TIRFE_10_6MHZ},
    {QT_TR_NOOP("12.4MHz"),TIRFE_12_4MHZ},
    {QT_TR_NOOP("14.7MHz"),TIRFE_14_7MHZ},
    {QT_TR_NOOP("17.6MHz"),TIRFE_17_6MHZ},
    {QT_TR_NOOP("21.0MHz"),TIRFE_21_0MHZ},
    {QT_TR_NOOP("21.4MHz"),TIRFE_21_4MHZ},
    {NULL,0}
};

/** \brief IF gain 2 options */
static const COMBO_ITEM_STRUCT _cisIFGain2[]=
{
    {QT_TR_NOOP("0dB"),TIG2E_P0_0DB},
    {QT_TR_NOOP("+3dB"),TIG2E_P3_0DB},
    {QT_TR_NOOP("+6dB"),TIG2E_P6_0DB},
    {QT_TR_NOOP("+9dB"),TIG2E_P9_0DB},
    {NULL,0}
};

/** \brief IF gain 3 options */
static const COMBO_ITEM_STRUCT _cisIFGain3[]=
{
    {QT_TR_NOOP("0dB"),TIG3E_P0_0DB},
    {QT_TR_NOOP("+3dB"),TIG3E_P3_0DB},
    {QT_TR_NOOP("+6dB"),TIG3E_P6_0DB},
    {QT_TR_NOOP("+9dB"),TIG3E_P9_0DB},
    {NULL,0}
};

/** \brief IF gain 4 options */
static const COMBO_ITEM_STRUCT _cisIFGain4[]=
{
    {QT_TR_NOOP("0dB"),TIG4E_P0_0DB},
    {QT_TR_NOOP("+1dB"),TIG4E_P1_0DB},
    {QT_TR_NOOP("+2dB"),TIG4E_P2_0DB},
    {NULL,0}
};

/** \brief IF filter options */
static const COMBO_ITEM_STRUCT _cisIFFilter[]=
{
    {QT_TR_NOOP("2.15MHz"),TIFE_2_15MHZ},
    {QT_TR_NOOP("2.20MHz"),TIFE_2_20MHZ},
    {QT_TR_NOOP("2.24MHz"),TIFE_2_24MHZ},
    {QT_TR_NOOP("2.28MHz"),TIFE_2_28MHZ},
    {QT_TR_NOOP("2.30MHz"),TIFE_2_30MHZ},
    {QT_TR_NOOP("2.40MHz"),TIFE_2_40MHZ},
    {QT_TR_NOOP("2.45MHz"),TIFE_2_45MHZ},
    {QT_TR_NOOP("2.50MHz"),TIFE_2_50MHZ},
    {QT_TR_NOOP("2.55MHz"),TIFE_2_55MHZ},
    {QT_TR_NOOP("2.60MHz"),TIFE_2_60MHZ},
    {QT_TR_NOOP("2.70MHz"),TIFE_2_70MHZ},
    {QT_TR_NOOP("2.75MHz"),TIFE_2_75MHZ},
    {QT_TR_NOOP("2.80MHz"),TIFE_2_80MHZ},
    {QT_TR_NOOP("2.90MHz"),TIFE_2_90MHZ},
    {QT_TR_NOOP("2.95MHz"),TIFE_2_95MHZ},
    {QT_TR_NOOP("3.00MHz"),TIFE_3_00MHZ},
    {QT_TR_NOOP("3.10MHz"),TIFE_3_10MHZ},
    {QT_TR_NOOP("3.20MHz"),TIFE_3_20MHZ},
    {QT_TR_NOOP("3.30MHz"),TIFE_3_30MHZ},
    {QT_TR_NOOP("3.40MHz"),TIFE_3_40MHZ},
    {QT_TR_NOOP("3.60MHz"),TIFE_3_60MHZ},
    {QT_TR_NOOP("3.70MHz"),TIFE_3_70MHZ},
    {QT_TR_NOOP("3.80MHz"),TIFE_3_80MHZ},
    {QT_TR_NOOP("3.90MHz"),TIFE_3_90MHZ},
    {QT_TR_NOOP("4.10MHz"),TIFE_4_10MHZ},
    {QT_TR_NOOP("4.30MHz"),TIFE_4_30MHZ},
    {QT_TR_NOOP("4.40MHz"),TIFE_4_40MHZ},
    {QT_TR_NOOP("4.60MHz"),TIFE_4_60MHZ},
    {QT_TR_NOOP("4.80MHz"),TIFE_4_80MHZ},
    {QT_TR_NOOP("5.00MHz"),TIFE_5_00MHZ},
    {QT_TR_NOOP("5.30MHz"),TIFE_5_30MHZ},
    {QT_TR_NOOP("5.50MHz"),TIFE_5_50MHZ},
    {NULL,0}
};

/** \brief IF gain 5 options */
static const COMBO_ITEM_STRUCT _cisIFGain5[]=
{
    {QT_TR_NOOP("+3dB"),TIG5E_P3_0DB},
    {QT_TR_NOOP("+6dB"),TIG5E_P6_0DB},
    {QT_TR_NOOP("+9dB"),TIG5E_P9_0DB},
    {QT_TR_NOOP("+12dB"),TIG5E_P12_0DB},
    {QT_TR_NOOP("+15dB"),TIG5E_P15_0DB},
    {NULL,0}
};

/** \brief IF gain 6 options */
static const COMBO_ITEM_STRUCT _cisIFGain6[]=
{
    {QT_TR_NOOP("+3dB"),TIG6E_P3_0DB},
    {QT_TR_NOOP("+6dB"),TIG6E_P6_0DB},
    {QT_TR_NOOP("+9dB"),TIG6E_P9_0DB},
    {QT_TR_NOOP("+12dB"),TIG6E_P12_0DB},
    {QT_TR_NOOP("+15dB"),TIG6E_P15_0DB},
    {NULL,0}
};


/** \brief List of all combo boxes. */    /** FIXME: Review defaults */
static COMBO_STRUCT _acs[] =
{
    {FCD_CMD_APP_SET_LNA_GAIN,     FCD_CMD_APP_GET_LNA_GAIN,    10, NULL, _cisLNAGain},
    {FCD_CMD_APP_SET_LNA_ENHANCE,  FCD_CMD_APP_GET_LNA_ENHANCE,  0, NULL, _cisLNAEnhance},
    {FCD_CMD_APP_SET_BAND,         FCD_CMD_APP_GET_BAND,         0, NULL, _cisBand},
    {FCD_CMD_APP_SET_RF_FILTER,    FCD_CMD_APP_GET_RF_FILTER,    0, NULL, _cisRFFilter1},
    {FCD_CMD_APP_SET_MIXER_GAIN,   FCD_CMD_APP_GET_MIXER_GAIN,   1, NULL, _cisMixerGain},
    {FCD_CMD_APP_SET_BIAS_CURRENT, FCD_CMD_APP_GET_BIAS_CURRENT, 3, NULL, _cisBiasCurrent},
    {FCD_CMD_APP_SET_MIXER_FILTER, FCD_CMD_APP_GET_MIXER_FILTER, 0, NULL, _cisMixerFilter},
    {FCD_CMD_APP_SET_IF_GAIN1,     FCD_CMD_APP_GET_IF_GAIN1,     1, NULL, _cisIFGain1},
    {FCD_CMD_APP_SET_IF_GAIN_MODE, FCD_CMD_APP_GET_IF_GAIN_MODE, 0, NULL, _cisIFGainMode},
    {FCD_CMD_APP_SET_IF_RC_FILTER, FCD_CMD_APP_GET_IF_RC_FILTER, 0, NULL, _cisIFRCFilter},
    {FCD_CMD_APP_SET_IF_GAIN2,     FCD_CMD_APP_GET_IF_GAIN2,     0, NULL, _cisIFGain2},
    {FCD_CMD_APP_SET_IF_GAIN3,     FCD_CMD_APP_GET_IF_GAIN3,     0, NULL, _cisIFGain3},
    {FCD_CMD_APP_SET_IF_GAIN4,     FCD_CMD_APP_GET_IF_GAIN4,     0, NULL, _cisIFGain4},
    {FCD_CMD_APP_SET_IF_FILTER,    FCD_CMD_APP_GET_IF_FILTER,    0, NULL, _cisIFFilter},
    {FCD_CMD_APP_SET_IF_GAIN5,     FCD_CMD_APP_GET_IF_GAIN5,     0, NULL, _cisIFGain5},
    {FCD_CMD_APP_SET_IF_GAIN6,     FCD_CMD_APP_GET_IF_GAIN6,     0, NULL, _cisIFGain6},
    {0, 0, 0, NULL, NULL}
};

static QString CBlabels[] = {
    "LNA gain", /*FCD_CMD_APP_SET_LNA_GAIN*/
    "LNA Enhance", /*FCD_CMD_APP_SET_LNA_ENHANCE*/
    "LNA Band", /*FCD_CMD_APP_SET_BAND*/
    "RF Filter", /*FCD_CMD_APP_SET_RF_FILTER*/
    "Mixer gain", /*FCD_CMD_APP_SET_MIXER_GAIN*/
    "Bias current", /*FCD_CMD_APP_SET_BIAS_CURRENT*/
    "Mixer filter", /*FCD_CMD_APP_SET_MIXER_FILTER*/
    "IF gain 1",
    "Gain mode", /* FCD_CMD_APP_SET_IF_GAIN_MODE */
    "IF RC filter",
    "IF gain 2",
    "IF gain 3",
    "IF gain 4",
    "IF filter",
    "IF gain 5",
    "IF gain 6"
} ;


FCDWidget::FCDWidget( FUNCube *device, QWidget *parent) :
    QWidget(parent)
{
    QGridLayout *plugin_layout = new QGridLayout( this );
    plugin_layout->setAlignment( Qt::AlignHCenter );
    plugin_layout->setContentsMargins( 0, 0, 0, 0 );

    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, QColor(0x60,0x60,0x60,0xFF));
    setAutoFillBackground(true);
    setPalette(Pal);

    //connect( cb_band, SIGNAL(currentIndexChanged(int)), this, SLOT(SLOT_selectBand(int)));

    comboBoxLNAGain = new QComboBox();
    comboBoxLNAEnhance = new QComboBox();
    comboBoxBand = new QComboBox();
    comboBoxRfFilter = new QComboBox();
    comboBoxMixerGain = new QComboBox();
    comboBoxBiasCurrent = new QComboBox();
    comboBoxMixerFilter = new QComboBox();
    comboBoxIFGain1 = new QComboBox();
    comboBoxIFGainMode = new QComboBox();
    comboBoxIFRCFilter = new QComboBox();
    comboBoxIFGain2 = new QComboBox();
    comboBoxIFGain3 = new QComboBox();
    comboBoxIFGain4 = new QComboBox();
    comboBoxIFFilter = new QComboBox();
    comboBoxIFGain5 = new QComboBox();
    comboBoxIFGain6 = new QComboBox();

    /* Populate Combo Box list structure */
    _acs[0].pComboBox = comboBoxLNAGain;
    _acs[1].pComboBox= comboBoxLNAEnhance;
    _acs[2].pComboBox= comboBoxBand;
    _acs[3].pComboBox= comboBoxRfFilter;
    _acs[4].pComboBox= comboBoxMixerGain;
    _acs[5].pComboBox= comboBoxBiasCurrent;
    _acs[6].pComboBox= comboBoxMixerFilter;
    _acs[7].pComboBox= comboBoxIFGain1;
    _acs[8].pComboBox= comboBoxIFGainMode;
    _acs[9].pComboBox= comboBoxIFRCFilter;
    _acs[10].pComboBox= comboBoxIFGain2;
    _acs[11].pComboBox= comboBoxIFGain3;
    _acs[12].pComboBox= comboBoxIFGain4;
    _acs[13].pComboBox= comboBoxIFFilter;
    _acs[14].pComboBox= comboBoxIFGain5;
    _acs[15].pComboBox= comboBoxIFGain6;

    int lbl = 0 ;
    int rc = 0 ;
    COMBO_STRUCT *pcs = _acs;
    while (pcs->pacis!=NULL)
    {

        const COMBO_ITEM_STRUCT *pcis = pcs->pacis;
        populateCombo(pcs->pComboBox, pcs->nIdxDefault, pcis);

        QVBoxLayout *box = new QVBoxLayout();
        QLabel *mt = new QLabel();
        mt->setStyleSheet("QLabel { color : yellow; }");
        mt->setText( CBlabels[lbl++]);
        box->addWidget( mt );
        box->addWidget( pcs->pComboBox );
        QWidget *wbox = new QWidget() ;
        wbox->setLayout( box );
        plugin_layout->addWidget( wbox, rc%8, rc/8, 1, 1);
        pcs++;
        rc++ ;
    }

    connect( comboBoxLNAGain, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxLNAGain_activated(int)));
    connect( comboBoxLNAEnhance, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxLNAEnhance_activated(int)));
    connect( comboBoxBand, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxRfFilter_activated(int)));
    connect( comboBoxRfFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxBiasCurrent_activated(int)));
    connect( comboBoxMixerGain, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxMixerGain_activated(int)));
    connect( comboBoxBiasCurrent, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxBiasCurrent_activated(int)));
    connect( comboBoxMixerFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxMixerFilter_activated(int)));
    connect( comboBoxIFGain1, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxIFGain1_activated(int)));
    connect( comboBoxIFGainMode, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxIFGainMode_activated(int)));
    connect( comboBoxIFRCFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxIFRCFilter_activated(int)));
    connect( comboBoxIFGain2, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxIFGain2_activated(int)));
    connect( comboBoxIFGain3, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxIFGain3_activated(int)));
    connect( comboBoxIFGain4, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxIFGain4_activated(int)));
    connect( comboBoxIFFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxIFFilter_activated(int)));
    connect( comboBoxIFGain5, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxIFGain5_activated(int)));
    connect( comboBoxIFGain6, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxIFGain6_activated(int)));

    this->device = device ;
}


/** \brief Populates a combo box with all its items and selects the default item
  * \param box Pointer to the QComboBox to populate.
  * \param nIdxDefault Index of the default item.
  * \param pcis Pointer to the array containing the data for the combo box items
  */
void FCDWidget::populateCombo(QComboBox *box, int nIdxDefault, const COMBO_ITEM_STRUCT *pcis)
{
    box->clear();
    while(pcis->pszDesc != NULL){
        box->addItem(QString(pcis->pszDesc));
        pcis++;
    }
    box->setCurrentIndex(nIdxDefault);
}


/** \brief Populate combo boxes
  *
  */
void FCDWidget::populateCombos()
{
    COMBO_STRUCT *pcs = _acs;

    /* iterate trough all combo boxes */
    while (pcs->pacis!=NULL)
    {
        const COMBO_ITEM_STRUCT *pcis = pcs->pacis;
        populateCombo(pcs->pComboBox, pcs->nIdxDefault, pcis);
        pcs++;
    }
}



void FCDWidget::on_comboBoxLNAGain_activated(int index)
{
    quint8 u8Write = _acs[0].pacis[index].u8Val;
    fcdAppSetParam(_acs[0].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxLNAEnhance_activated(int index)
{
    quint8 u8Write = _acs[1].pacis[index].u8Val;
    fcdAppSetParam(_acs[1].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxRfFilter_activated(int index)
{
    quint8 u8Write = _acs[3].pacis[index].u8Val;
    fcdAppSetParam(_acs[3].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxMixerGain_activated(int index)
{
    quint8 u8Write = _acs[4].pacis[index].u8Val;
    fcdAppSetParam(_acs[4].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxBiasCurrent_activated(int index)
{
    quint8 u8Write = _acs[5].pacis[index].u8Val;
    fcdAppSetParam(_acs[5].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxMixerFilter_activated(int index)
{
    quint8 u8Write = _acs[6].pacis[index].u8Val;
    fcdAppSetParam(_acs[6].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxIFGain1_activated(int index)
{
    quint8 u8Write = _acs[7].pacis[index].u8Val;
    fcdAppSetParam(_acs[7].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxIFGainMode_activated(int index)
{
    quint8 u8Write = _acs[8].pacis[index].u8Val;
    fcdAppSetParam(_acs[8].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxIFRCFilter_activated(int index)
{
    quint8 u8Write = _acs[9].pacis[index].u8Val;
    fcdAppSetParam(_acs[9].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxIFGain2_activated(int index)
{
    quint8 u8Write = _acs[10].pacis[index].u8Val;
    fcdAppSetParam(_acs[10].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxIFGain3_activated(int index)
{
    quint8 u8Write = _acs[11].pacis[index].u8Val;
    fcdAppSetParam(_acs[11].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxIFGain4_activated(int index)
{
    quint8 u8Write = _acs[12].pacis[index].u8Val;
    fcdAppSetParam(_acs[12].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxIFFilter_activated(int index)
{
    quint8 u8Write = _acs[13].pacis[index].u8Val;
    fcdAppSetParam(_acs[13].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxIFGain5_activated(int index)
{
    quint8 u8Write = _acs[14].pacis[index].u8Val;
    fcdAppSetParam(_acs[14].u8CommandSet, &u8Write, 1);
}

void FCDWidget::on_comboBoxIFGain6_activated(int index)
{
    quint8 u8Write = _acs[15].pacis[index].u8Val;
    fcdAppSetParam(_acs[15].u8CommandSet,&u8Write,1);
}


void FCDWidget::fcdAppSetParam(unsigned char u8Cmd, unsigned char *pu8Data, unsigned char u8len) {
    //qDebug() << "FCDWidget::fcdAppSetParam" <<  u8Cmd ;
    if( device != NULL ) {
        device->fcdAppSetParam(u8Cmd,pu8Data,u8len);
    }
}
