/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecov.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \mainpage Статическая библиотека простенького 3D движка.

 Движок позволяет задавать несколько окон с трехмерными мирами. Добавлять в миры различные элементы,
 управлять их характеристиками через указатели на объекты. Изменять характеристики самих миров.

 Принцип работы с библиотекой очень прост. Создаётся объекты типа \a VasnecovUniverse и \a VasnecovScene
 (для расширения возможностей можно отнаследоваться от VasnecovScene). Работа со сценой аналогична работой
 с QGraphicsScene. Методом \a VasnecovScene::setUniverse сцене присваивается Вселенная.

 Затем с помощью объекта Вселенной создаются необходимые Миры \a VasnecovWorld и элементы, которые в
 них размещаются. При их создании Вселенная возвращает указатель на соответствующий объект.

 Управления мирами и элементами производится через эти указатели.

 Библиотека рассчитана на многопоточную работу. Отрисовка происходит по вызову метода VasnecovScene::drawBackground,
 который может быть вызван как самой Qt-библиотекой, так и непосредственно. Данный вызов осуществляется
 в основном потоке работы приложения.

 Создание новых элементов и изменение их характеристик происходит в других потоках. Все вызовы используют
 мьютексы для защиты данных. Ресурсоёмкие операции (такие, как загрузка файлов моделей и текстур, расчет
 точек для фигур, рекурсивное позиционирование, удаление объектов и т.п.) как правило выполняются вне
 основного потока.

 \sa VasnecovScene
 \sa VasnecovUniverse
 \sa VasnecovWorld
*/

Vasnecov::Version vVersion;

/*!
 \brief Выдает номер версии библиотеки (в виде строки).

 \return QString строка с номером версии.
*/
QString showVasnecovVersion()
{
    return vVersion.versionText;
}
/*!
 \brief Выдает номер версии библиотеки.

 \return Vasnecov::Version номер версии в своём формате.
*/
Vasnecov::Version vasnecovVersion()
{
    return vVersion;
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
