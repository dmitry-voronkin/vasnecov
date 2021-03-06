/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovscene.h"
#include "vasnecovuniverse.h"
#include <QPainter>
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \brief

 \fn VasnecovScene::VasnecovScene
 \param parent
*/
VasnecovScene::VasnecovScene(QObject *parent) :
    QGraphicsScene(parent),
    m_width(0),
    m_height(0),
    m_universe(0)
{
}

/*!
 \brief

 \fn VasnecovScene::drawBackground
 \param painter
 \param
*/
void VasnecovScene::drawBackground(QPainter *painter, const QRectF &)
{
    m_width = painter->device()->width();
    m_height = painter->device()->height();
    painter->beginNativePainting();
    if(m_universe)
    {
        m_universe->renderDrawAll(m_width, m_height);
    }
}

/*!
 \brief

 \fn VasnecovScene::setUniverse
 \param universe
*/
void VasnecovScene::setUniverse(VasnecovUniverse *universe)
{
    if(universe)
    {
        m_universe = universe;
        m_universe->renderInitialize();
    }
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
