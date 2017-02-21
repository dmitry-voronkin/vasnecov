#include "vasnecovpipeline.h"
#include <algorithm>
#include <QGLContext>
#include "configuration.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
  \class VasnecovPipeline
  \brief Обёртка вычислительного конвейера OpenGL.

  Класс управляет и контролирует состояния конвейера. По сути, является прослойкой для абстракции вызовов
  OpenGL. Если задаваемый параметр уже имеет такое же значение, то его состояния не меняется, вызова
  функций OpenGL не происходит.
  */

/*!
 \brief Конструктор конвейера.

 Производит базовые инициализации. Установка начального состония выполняется методом \a initialize()
*/
VasnecovPipeline::VasnecovPipeline(QGLContext *context) :
    m_context(context),
    m_backgroundColor(0, 0, 0, 255),
    m_color(255, 255, 255, 255),
    m_drawingType(Vasnecov::PolygonDrawingTypeNormal),
    m_texture2D(0),
    m_P(),
    m_viewX(0),
    m_viewY(0),
    m_viewWidth(Vasnecov::cfg_displayWidthDefault),
    m_viewHeight(Vasnecov::cfg_displayHeightDefault),
    m_activatedLamps(),

    m_flagTexture2D(false),
    m_flagLight(false),
    m_flagDepth(true),
    m_materialColoring(true),
    m_backFaces(false),
    m_blending(true),
    m_smoothShading(true),

    m_ambientColor(51, 51, 51, 255),
    m_materialColoringType(AmbientAndDiffuse),
    m_materialColorAmbient(51, 51, 51, 255),
    m_materialColorDiffuse(204, 204, 204, 255),
    m_materialColorSpecular(0, 0, 0, 255),
    m_materialColorEmission(0, 0, 0, 255),
    m_materialShininess(0),

    m_lineWidth(1.0f),
    m_pointSize(1.0f),

    m_wasSomethingUpdated(true)

//	m_config()
{
    m_activatedLamps.reserve(8); // Минимальное количество источников в OpenGL
}

/*!
 \brief Начальная инициализация состояний конвейера

 Выставляет определенные значения состояний вычислительного конвейера OpenGL. Данный метод должен
 быть вызван перед вызовами любых других методов конвейера.
*/
void VasnecovPipeline::initialize(QGLContext *context)
{
    m_context = context;

    glDisable(GL_LIGHTING);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    glDepthFunc(GL_LESS); // Тип теста глубины
    glEnable(GL_DEPTH_TEST); // Разрешить тест глубины

    glShadeModel(GL_SMOOTH); // Разрешить плавное цветовое сглаживание
    glEnable(GL_POINT_SMOOTH); // сглаживание точек // NOTE: it will be deprecated

    glEnable(GL_BLEND); // Включение прозрачности (смешивания)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Альфа-прозрачность
#ifndef _MSC_VER
    glBlendEquation(GL_FUNC_ADD);
//	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
#endif

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE); // выключить задние грани

    glDisable(GL_TEXTURE_2D);

//	glColorMaterial(GL_FRONT, m_materialColoringType); // Рассеянный и дифузный задаются через glColor
    glEnable(GL_COLOR_MATERIAL); // Включить раскраску с помощью glColor

    glLineWidth(m_lineWidth);
    glPointSize(m_pointSize);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/*!
 \brief

 \fn VasnecovPipeline::setPerspective
 \param perspective
 \param camera
*/
void VasnecovPipeline::setPerspective(const Vasnecov::Perspective &perspective, const CameraAttributes &camera)
{
    glMatrixMode(GL_PROJECTION);

    m_P.setToPerspective(perspective.angle, perspective.ratio, perspective.frontBorder, perspective.backBorder);
    setCamera(camera);

    glLoadMatrixf(m_P.constData());

    glMatrixMode(GL_MODELVIEW);
}
/*!
 \brief

 \fn VasnecovPipeline::setOrtho
 \param ortho
 \param camera
*/
void VasnecovPipeline::setOrtho(const Vasnecov::Ortho &ortho, const CameraAttributes &camera)
{
    glMatrixMode(GL_PROJECTION);

    m_P.setToOrtho(ortho.left, ortho.right, ortho.bottom, ortho.top, ortho.front, ortho.back);
    setCamera(camera);

    glLoadMatrixf(m_P.constData());

    glMatrixMode(GL_MODELVIEW);
}

/*!
 \brief

 \fn VasnecovPipeline::setPerspective
 \param perspective
*/
void VasnecovPipeline::setPerspective(const Vasnecov::Perspective &perspective)
{
    glMatrixMode(GL_PROJECTION);

    m_P.setToPerspective(perspective.angle, perspective.ratio, perspective.frontBorder, perspective.backBorder);
    glLoadMatrixf(m_P.constData());

    glMatrixMode(GL_MODELVIEW);
}
/*!
 \brief

 \fn VasnecovPipeline::setOrtho
 \param ortho
*/
void VasnecovPipeline::setOrtho(const Vasnecov::Ortho &ortho)
{
    glMatrixMode(GL_PROJECTION);

    m_P.setToOrtho(ortho.left, ortho.right, ortho.bottom, ortho.top, ortho.front, ortho.back);
    glLoadMatrixf(m_P.constData());

    glMatrixMode(GL_MODELVIEW);
}

/*!
 \brief Включает "технологический" режим двухмерной отрисовки

 Данный метод меняет матрицу проектирования на ортогональную для вывода непосредственно на плоскость
 экрана. Вывод осуществляется с учетом установленного окна просмотра. Глубина задается в диапазоне
 [-1; 1].

 Режим является технологическим, потому что предыдущая матрица проектирования сохраняется внутри конвейера
 и может быть использована в последующих вычислениях проекций. Возврат к предыдущему состоянию производится
 вызовом \a unsetOrtho2D()

 \sa setViewport()
*/
void VasnecovPipeline::setOrtho2D()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, m_viewWidth, 0, m_viewHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

void VasnecovPipeline::unsetOrtho2D()
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_P.constData());

    glMatrixMode(GL_MODELVIEW);
}
/*!
 \brief Задает положение и размер окна просмотра

 \param x горизонтальное положение левой нижней точки
 \param y вертикальное положение левой нижней точки
 \param width ширина окна
 \param height высота окна
*/
void VasnecovPipeline::setViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    m_viewX = x;
    m_viewY = y;
    m_viewWidth = width;
    m_viewHeight = height;
    glViewport(m_viewX, m_viewY, m_viewWidth, m_viewHeight);
}
/*!
   \brief VasnecovPipeline::setMatrixOrtho2D
   \param MV
 */
void VasnecovPipeline::setMatrixOrtho2D(const VasnecovMatrix4x4 &MV)
{
    GLmatrix matrix;
    matrix.setColumn(3, projectPoint(MV));

    glLoadMatrixf(matrix.constData());
}
/*!
   \brief Вычисление проекции точки на плоскость экрана.

   Принцип действия аналогичен gluProject(). Основное отличие в том, что используется фактическая
   матрица проектирования без учета вызова метода \a setOrtho2D(). А результат
   пересчитывается не в координаты экрана, а в координаты окна просмотра, задаваемого \a setViewport() .

   Координата глубины выдаётся в диапазоне значений [-1; 1].

   \param MV матрица модельно-видовых преобразований
   \param point кординаты точки в трехмерном пространстве
   \return QVector4D координаты точки на плоскости экрана

   \sa setOrtho2D()
   \sa setViewport()
   \sa setMatrixOrtho2D()
 */
QVector4D VasnecovPipeline::projectPoint(const GLmatrix &MV, const QVector3D &point)
{
    QVector4D wPoint(point);
    wPoint.setW(1.0f);

    QVector4D pos = m_P * MV * wPoint;

    // Приведение вектора к нормализованным координатам
    if(fabs(pos.w()) != 1.0f)
    {
        GLfloat div = 1.0f/pos.w();
        pos.setX(pos.x()*div);
        pos.setY(pos.y()*div);
        pos.setZ(pos.z()*div);
        pos.setW(1.0f);
    }
    pos.setX((pos.x() + 1)*0.5*m_viewWidth);
    pos.setY((pos.y() + 1)*0.5*m_viewHeight);

    return pos;
}
/*!
 \brief

 \fn VasnecovPipeline::setColor
 \param color
*/
void VasnecovPipeline::setColor(const QColor &color)
{
    if(color != m_color)
    {
        m_color = color;
        glColor4f(m_color.redF(), m_color.greenF(), m_color.blueF(), m_color.alphaF());
    }
}
/*!
 \brief

 \fn VasnecovPipeline::setAmbientColor
 \param color
*/
void VasnecovPipeline::setAmbientColor(const QColor &color)
{
    if(color != m_ambientColor)
    {
        m_ambientColor = color;
        GLfloat params[4];
        params[0] = m_ambientColor.redF();
        params[1] = m_ambientColor.greenF();
        params[2] = m_ambientColor.blueF();
        params[3] = m_ambientColor.alphaF();
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, params);
    }
}

/*!
 \brief

 \fn VasnecovPipeline::disableAllConcreteLamps
 \param strong
*/
void VasnecovPipeline::disableAllConcreteLamps(GLboolean strong)
{
    if(!strong)
    {
        for(std::vector<GLuint>::iterator lit = m_activatedLamps.begin();
            lit != m_activatedLamps.end(); ++lit)
        {
            glDisable(*lit);
        }
        m_activatedLamps.clear();
    }
    else
    {
        for(GLuint i = GL_LIGHT0; i < Vasnecov::cfg_lampsCountMax; ++i)
        {
            glDisable(i);
        }
    }
}

/*!
 \brief

 \fn VasnecovPipeline::setMaterialColors
 \param ambient
 \param diffuse
 \param specular
 \param emission
 \param shininess
*/
void VasnecovPipeline::setMaterialColors(const QColor &ambient, const QColor &diffuse, const QColor &specular, const QColor &emission, GLfloat shininess)
{
    if(m_materialColoring)
    {
        switch(m_materialColoringType)
        {
            case AmbientAndDiffuse:
                setColor(ambient);
                setMaterialSpecularColor(specular);
                setMaterialEmissionColor(emission);
                break;
            case Ambient:
                setColor(ambient);
                setMaterialDiffuseColor(diffuse);
                setMaterialSpecularColor(specular);
                setMaterialEmissionColor(emission);
                break;
            case Diffuse:
                setMaterialAmbientColor(ambient);
                setColor(diffuse);
                setMaterialSpecularColor(specular);
                setMaterialEmissionColor(emission);
                break;
            case Specular:
                setMaterialAmbientColor(ambient);
                setMaterialDiffuseColor(diffuse);
                setColor(specular);
                setMaterialEmissionColor(emission);
                break;
            case Emission:
                setMaterialAmbientColor(ambient);
                setMaterialDiffuseColor(diffuse);
                setMaterialSpecularColor(specular);
                setColor(emission);
                break;
            default:;
        }
    }
    else
    {
        setMaterialAmbientColor(ambient);
        setMaterialDiffuseColor(diffuse);
        setMaterialSpecularColor(specular);
        setMaterialEmissionColor(emission);
    }

    setMaterialShininess(shininess);
}
/*!
 \brief

 \fn VasnecovPipeline::applyMaterialColors
*/
void VasnecovPipeline::applyMaterialColors()
{
    // From Qt3D setMaterial()
    GLfloat params[16];

    params[0] = m_materialColorAmbient.redF();
    params[1] = m_materialColorAmbient.greenF();
    params[2] = m_materialColorAmbient.blueF();
    params[3] = m_materialColorAmbient.alphaF();

    params[4] = m_materialColorDiffuse.redF();
    params[5] = m_materialColorDiffuse.greenF();
    params[6] = m_materialColorDiffuse.blueF();
    params[7] = m_materialColorDiffuse.alphaF();

    params[8] = m_materialColorSpecular.redF();
    params[9] = m_materialColorSpecular.greenF();
    params[10] = m_materialColorSpecular.blueF();
    params[11] = m_materialColorSpecular.alphaF();

    params[12] = m_materialColorEmission.redF();
    params[13] = m_materialColorEmission.greenF();
    params[14] = m_materialColorEmission.blueF();
    params[15] = m_materialColorEmission.alphaF();

    glMaterialfv(m_face, GL_AMBIENT, params);
    glMaterialfv(m_face, GL_DIFFUSE, params + 4);
    glMaterialfv(m_face, GL_SPECULAR, params + 8);
    glMaterialfv(m_face, GL_EMISSION, params + 12);
    glMaterialf(m_face, GL_SPECULAR, m_materialShininess);
}

/*!
 \brief

 \fn VasnecovPipeline::setCamera
 \param camera
*/
void VasnecovPipeline::setCamera(const CameraAttributes &camera)
{
    // Преобразования камеры
    // Вопреки здравому смыслу, камера задаётся в матрицу проекции, а не в видовую. Это позволяет применять трансформации
    // объектов прямым заданием матрицы (через glLoadMatrix), без использования стека видово-модельных матриц

    m_P.lookAt(camera.eye, camera.center, camera.up);
}
void VasnecovPipeline::drawElements(VasnecovPipeline::ElementDrawingMethods method,
                                    const std::vector<GLuint> *indices,
                                    const std::vector<QVector3D> *vertices,
                                    const std::vector<QVector3D> *normals,
                                    const std::vector<QVector2D> *textures) const
{
    if(indices && vertices)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices->data());
        if(normals && !normals->empty())
        {
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, 0, normals->data());
        }
        if(textures && !textures->empty())
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, textures->data());
        }

        glDrawElements(method, indices->size(), GL_UNSIGNED_INT, indices->data());

        if(textures)
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if(normals)
        {
            glDisableClientState(GL_NORMAL_ARRAY);
        }
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif