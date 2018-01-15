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

#ifndef FCDWIDGET_H
#define FCDWIDGET_H

#include <QWidget>
#include <QPalette>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QColor>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include "funcube.h"

/** \brief Data definition for combo box items.
  * This data structure represents an item in a combo box used for
  * gain and filter parameters. Each combo box has an array of these
  * data defining the set of options.
  */
typedef struct {
    const char *pszDesc;  /*!< The display name of the item. */
    qint8 u8Val;          /*!< The numerical value of the item. */
} COMBO_ITEM_STRUCT;


/** \brief Data definition for a combo box.
  * This data structure represents a combo box that is used for gain and
  * filter parameters. Each such parameter has a 'set' and 'get' command
  * as well as a list of options (COMBO_ITEM_STRUCT[]).
  * \sa _acs
  */
typedef struct {
    quint8 u8CommandSet;   /*!< The command for setting the parameter in the FCD. */
    quint8 u8CommandGet;   /*!< The command for retrieveing the parameter from the FCD. */
    qint8 nIdxDefault;    /*!< Index pointing to the default value for this parameter in the pacis array. */
    QComboBox *pComboBox; /*!< Pointer to the combo box. */
    const COMBO_ITEM_STRUCT *pacis; /*!< Pointer to the array of items. */
} COMBO_STRUCT;




class FCDWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FCDWidget( FUNCube *device, QWidget *parent = 0);

signals:

public slots:

private slots:
    void on_comboBoxLNAGain_activated(int index);
    void on_comboBoxRfFilter_activated(int index);
    void on_comboBoxMixerGain_activated(int index);
    void on_comboBoxMixerFilter_activated(int index);
    void on_comboBoxIFGain1_activated(int index);
    void on_comboBoxIFRCFilter_activated(int index);
    void on_comboBoxIFGain2_activated(int index);
    void on_comboBoxIFGain3_activated(int index);
    void on_comboBoxIFFilter_activated(int index);
    void on_comboBoxIFGain4_activated(int index);
    void on_comboBoxIFGain5_activated(int index);
    void on_comboBoxIFGain6_activated(int index);
    void on_comboBoxLNAEnhance_activated(int index);
    void on_comboBoxBiasCurrent_activated(int index);
    void on_comboBoxIFGainMode_activated(int index);

private:
    void populateCombo(QComboBox *box, int nIdxDefault, const COMBO_ITEM_STRUCT *pcis);
    void populateCombos();
    void fcdAppSetParam(unsigned char u8Cmd, unsigned char *pu8Data, unsigned char u8len);

    FUNCube* device ;

    QComboBox *comboBoxLNAGain ;
    QComboBox *comboBoxLNAEnhance;
    QComboBox *comboBoxBand;
    QComboBox *comboBoxRfFilter;
    QComboBox *comboBoxMixerGain;
    QComboBox *comboBoxBiasCurrent;
    QComboBox *comboBoxMixerFilter;
    QComboBox *comboBoxIFGain1;
    QComboBox *comboBoxIFGainMode;
    QComboBox *comboBoxIFRCFilter;
    QComboBox *comboBoxIFGain2;
    QComboBox *comboBoxIFGain3;
    QComboBox *comboBoxIFGain4;
    QComboBox *comboBoxIFFilter;
    QComboBox *comboBoxIFGain5;
    QComboBox *comboBoxIFGain6;
};

#endif // FCDWIDGET_H
