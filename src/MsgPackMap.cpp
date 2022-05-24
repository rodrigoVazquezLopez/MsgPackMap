#include "MsgPackMap.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include <string.h>

/**
  *  @brief Constructor del objeto.
  *  @param buffer      Dirección de memoria del buffer de datos (arreglo de bytes).
  *  @param bufsize     Tamaño del buffer.
  *  @return none
  */
MsgPackMap::MsgPackMap(byte buffer[], uint16_t bufsize)
{
    this->buffer = buffer;
    bufferSize = bufsize;
    numElements = 0;
    startPos = 0;
    bufferPos = 0;
}

/**
  *  @brief Deveulve el tamaño en bytes de la estructura.
  *  @return int    Tamaño de la esttructura en bytes
  */
uint16_t MsgPackMap::getMapSize()
{
    if(bufferPos > 0)
        return bufferPos;
    else
        return 0; //aqui va el codigo para detener el map
}

/**
  *  @brief Asigna un objeto de tipo Stream para escribir en el. Esta función se debe llamar antes de utilizar
  *         writeDara() y printRawData().
  *  @param serial      Dirección de memoria del objeto Stream (monitor serial, etc.).
  *  @return none
  */
void MsgPackMap::setStream(Stream &serial)
{
    _serial = &serial;
}

/**
  *  @brief Devuelve el número de elementos que contiene la estructura.
  *  @return none
  */
uint8_t MsgPackMap::readNumElements()
{
    if((*(buffer) & 0xf0) == 0x80)
        return (*(buffer) & 0x0f);
    else if((*(buffer) == 0xde))
        return *(buffer+2);
    else
        return 0;
}

/**
  *  @brief Imprime el contenido del buffer en un objeto de tipo Serial ((Monitor serie,
  *         objeto de tipo software serial, etc). EL formato de salida es hexadecimal separado
  *         por espacios.
  *  @param serial      Dirección de memoria del objeto de tipo Stream.
  *  @return none
  */
void MsgPackMap::printRawData()
{
    for(int i=0;i<bufferPos;i++)
    {
        if(*(buffer+i) <= 0x0f)
        {
            _serial->print("0");
        }
        _serial->print(*(buffer+i),HEX);
        _serial->print(" ");
    }
    _serial->println();
}

/**
  *  @brief Imprime el contenido del buffer en un objeto de tipo Serial ((Monitor serie,
  *         objeto de tipo software serial, etc). EL formato de salida es hexadecimal separado
  *         por espacios y por columnas.
  *  @param serial      Dirección de memoria del objeto de tipo Stream.
  *  @param numCol      Número de columnas.
  *  @return none
  */
void MsgPackMap::printRawData(int numCol)
{
    for(int i=0;i<bufferPos;i++)
    {
        if(*(buffer+i) <= 0x0f)
            _serial->print("0");
        _serial->print(*(buffer+i),HEX);
        _serial->print(" ");
        if(numCol > 0)
        {
            if((i + 1)%numCol == 0)
                _serial->println();
        }
    }
}

/**
  *  @brief Escribe el contenido del buffer en un objeto de tipo Stream (Monitor serie,
  *         objeto de tipo software serial, etc).
  *  @param serial    Dirección de memoria del objeto de tipo Stream.
  *  @return none
  */
void MsgPackMap::writeData()
{
    for(int i=0; i<bufferPos; i++)
        _serial->write(*(buffer+bufferPos));
}

/**
  *  @brief Desplaza el contenido del buffer dos posiciones a la derecha a partir
  *         del valor de la variable startPos. Esta función se invoca cuando el numero
  *         de elementos del mapa o submapa es mayor a 15.
  *  @return none
  */
bool MsgPackMap::rearrageBuffer()
{
    if(bufferPos+3 < bufferSize)
    {
        for(int i=bufferPos-1; i>startPos; i--)
            *(buffer+(i+2)) = *(buffer+i);
        return true;
    }
    return false;
}

/**
  *  @brief Limpia el buffer e inicializa la estructura.
  *  @return none
  */
void MsgPackMap::clearData() //Limpia el buffer
{
    memset(buffer,0,bufferSize);
    numElements = 0;
    startPos = 0;
    bufferPos = 0;
    level = 0;
    for(int i=0;i<MAX_SUBMAPS;i++)
    {
        positions[i] = 0;
        elements[i] = 0;
    }
}

/*********************************************************************
  *
  *  Métodos para serializar los datos (data -> msgpack format)
  *
  ********************************************************************/

/**
  *  @brief Serializa y escribe en el buffer un entero de 8 bits sin signo.
  *  @param data    Entero de 8 bits sin signo.
  *  @return none
  */
void MsgPackMap::serializeInteger(uint8_t data)
{
    if(data > 127)
        *(buffer+(bufferPos++)) = 0xcc;
    *(buffer+(bufferPos++)) = data;
}

/**
  *  @brief Serializa y escribe en el buffer un entero de 16 bits sin signo.
  *  @param data    Entero de 16 bits sin signo.
  *  @return none
  */
void MsgPackMap::serializeInteger(uint16_t data)
{
    if(data <= 255)
    {
        serializeInteger((uint8_t)data);
    }
    else
    {
        *(buffer+(bufferPos++)) = 0xcd;
        *(buffer+(bufferPos++)) = (data >> 8);
        *(buffer+(bufferPos++)) = (data & 0xff);
    }
}

/**
  *  @brief Serializa y escribe en el buffer un entero de 32 bits sin signo.
  *  @param data    Entero de 32 bits sin signo.
  *  @return none
  */
void MsgPackMap::serializeInteger(uint32_t data)
{
    if(data <= 255)
    {
        serializeInteger((uint8_t)data);
    }
    else if(data <= 65535)
    {
        serializeInteger((uint16_t)data);
    }
    else
    {
        *(buffer+(bufferPos++)) = 0xce;
        *(buffer+(bufferPos++)) = (data >> 24);
        *(buffer+(bufferPos++)) = (data >> 16);
        *(buffer+(bufferPos++)) = (data >> 8);
        *(buffer+(bufferPos++)) = (data & 0xff);
    }
}

/**
  *  @brief Serializa y escribe en el buffer un entero de 8 bits con signo.
  *  @param data    Entero de 8 bits con signo.
  *  @return none
  */
void MsgPackMap::serializeInteger(int8_t data)
{
    if(data >= 0)
    {
        serializeInteger((uint8_t)data);
    }
    else
    {
        if(data < -32)
            *(buffer+(bufferPos++)) = 0xd0;
        *(buffer+(bufferPos++)) = data;
    }
}

/**
  *  @brief Serializa y escribe en el buffer un entero de 16 bits con signo.
  *  @param data    Entero de 16 bits con signo.
  *  @return none
  */
void MsgPackMap::serializeInteger(int16_t data)
{
    if(data >= 0)
    {
        serializeInteger((uint16_t)data);
    }
    else
    {
        if(data >= -128)
        {
            serializeInteger((int8_t)data);
        }
        else
        {
            *(buffer+(bufferPos++)) = 0xd1;
            *(buffer+(bufferPos++)) = (data >> 8);
            *(buffer+(bufferPos++)) = (data & 0xff);
        }
    }
}

/**
  *  @brief Serializa y escribe en el buffer un entero de 832bits con signo.
  *  @param data    Entero de 32 bits con signo.
  *  @return none
  */
void MsgPackMap::serializeInteger(int32_t data)
{
    if(data >= 0)
    {
        serializeInteger((uint32_t)data);
    }
    else
    {
        if(data >= -128)
        {
            serializeInteger((int8_t)data);
        }
        else if(data >= -32768)
        {
            serializeInteger((int16_t)data);
        }
        else
        {
            *(buffer+(bufferPos++)) = 0xd2;
            *(buffer+(bufferPos++)) = (data >> 24);
            *(buffer+(bufferPos++)) = (data >> 16);
            *(buffer+(bufferPos++)) = (data >> 8);
            *(buffer+(bufferPos++)) = (data & 0xff);
        }
    }
}

/**
  *  @brief Serializa y escribe en el buffer un número de punto flotante de 4 bytes.
  *  @param data    Número de punto flotante de 4 bytes.
  *  @return none
  */
void MsgPackMap::serializeFloat(float data)
{
    dec.num = data;
    *(buffer+(bufferPos++)) = 0xca;
    *(buffer+(bufferPos++)) = dec.numBytes[3];
    *(buffer+(bufferPos++)) = dec.numBytes[2];
    *(buffer+(bufferPos++)) = dec.numBytes[1];
    *(buffer+(bufferPos++)) = dec.numBytes[0];
}

/**
  *  @brief Serializa y escribe en el buffer una variable booleana.
  *  @param data    Valor booleano.
  *  @return none
  */
void MsgPackMap::serializeBool(bool data)
{
    *(buffer+(bufferPos++)) = data ? 0xc3 : 0xc2;
}

/**
  *  @brief Serializa y escribe en el buffer el valor nulo.
  *  @return none
  */
void MsgPackMap::serializeNil()
{
    *(buffer+(bufferPos++)) = 0xc0;
}

/**
  *  @brief Serializa y escribe en el buffer una cadena de caracteres.
  *  @param data    Cadena a escribir. Soporta los formatos fix str y str 8
  *                 (tamaño máximo de 256 caracteres).
  *  @return none
  */
void MsgPackMap::serializeString(const char data[])
{
    uint8_t dataSize = strlen(data);
    if(dataSize <= 32)
    {
        *(buffer+(bufferPos++)) = (0xa0 + dataSize);
    }
    else
    {
        *(buffer+(bufferPos++)) = 0xd9;
        *(buffer+(bufferPos++)) = dataSize;
    }
    for(int i=0;i<dataSize;i++)
        *(buffer+(bufferPos++)) = data[i];
}

/**
  *  @brief Serializa y escribe en el buffer un arreglo de bytes.
  *  @param data        Arreglo de bytes.
  *  @param dataSize    Tamaño del arreglo (hasta 256 elementos).
  *  @return none
  */
void MsgPackMap::serializeByte(byte data[],uint8_t dataSize)
{
    if(dataSize > 0)
    {
        *(buffer+(bufferPos++)) = 0xc4;
        *(buffer+(bufferPos++)) = dataSize;
        for(int i=0;i<dataSize;i++)
            *(buffer+(bufferPos++)) = data[i];
    }
}

/**
  *  @brief Serializa y escribe en el buffer un arreglo de números de punto flotante de 4 bytes.
  *  @param data        Arreglo de números de punto flotante.
  *  @param dataSize    Tamaño del arreglo (hasta 256 elementos).
  *  @return none
  */
void MsgPackMap::serializeFloatArray(float data[],uint8_t dataSize)
{
    if(dataSize > 0)
    {
        if(dataSize < 16)
        {
            *(buffer+(bufferPos++)) = 0x90 + dataSize;
        }
        else
        {
            *(buffer+(bufferPos++)) = 0xdc;
            *(buffer+(bufferPos++)) = 0x00;
            *(buffer+(bufferPos++)) = dataSize;
        }
        for(int i=0;i<dataSize;i++)
            serializeFloat(data[i]);
    }
}

/**
  *  @brief Serializa y escribe en el buffer un arreglo de enteros de 8 bits sin signo.
  *  @param data        Arreglo de enteros de 8 bits sin signo.
  *  @param dataSize    Tamaño del arreglo (hasta 256 elementos).
  *  @return none
  */
void MsgPackMap::serializeIntegerArray(uint8_t data[],uint8_t dataSize)
{
    if(dataSize > 0)
    {
        if(dataSize < 16)
        {
            *(buffer+(bufferPos++)) = 0x90 + dataSize;
        }
        else
        {
            *(buffer+(bufferPos++)) = 0xdc;
            *(buffer+(bufferPos++)) = 0x00;
            *(buffer+(bufferPos++)) = dataSize;
        }
        for(int i=0;i<dataSize;i++)
            serializeInteger(data[i]);
    }
}

/**
  *  @brief Serializa y escribe en el buffer un arreglo de enteros de 16 bits sin signo.
  *  @param data        Arreglo de enteros de 16 bits sin signo.
  *  @param dataSize    Tamaño del arreglo (hasta 256 elementos).
  *  @return none
  */
void MsgPackMap::serializeIntegerArray(uint16_t data[],uint8_t dataSize)
{
    if(dataSize > 0)
    {
        if(dataSize < 16)
        {
            *(buffer+(bufferPos++)) = 0x90 + dataSize;
        }
        else
        {
            *(buffer+(bufferPos++)) = 0xdc;
            *(buffer+(bufferPos++)) = 0x00;
            *(buffer+(bufferPos++)) = dataSize;
        }
        for(int i=0;i<dataSize;i++)
            serializeInteger(data[i]);
    }
}

/**
  *  @brief Serializa y escribe en el buffer un arreglo de enteros de 32 bits.
  *  @param data        Arreglo de enteros de 32 bits.
  *  @param dataSize    Tamaño del arreglo (hasta 256 elementos).
  *  @return none
  */
void MsgPackMap::serializeIntegerArray(uint32_t data[],uint8_t dataSize)
{
    if(dataSize > 0)
    {
        if(dataSize < 16)
        {
            *(buffer+(bufferPos++)) = 0x90 + dataSize;
        }
        else
        {
            *(buffer+(bufferPos++)) = 0xdc;
            *(buffer+(bufferPos++)) = 0x00;
            *(buffer+(bufferPos++)) = dataSize;
        }
        for(int i=0;i<dataSize;i++)
            serializeInteger(data[i]);
    }
}

/**
  *  @brief Serializa y escribe en el buffer un arreglo de enteros de 8 bits con signo.
  *  @param data        Arreglo de enteros de 8 bits con signo.
  *  @param dataSize    Tamaño del arreglo (hasta 256 elementos).
  *  @return none
  */
void MsgPackMap::serializeIntegerArray(int8_t data[],uint8_t dataSize)
{
    if(dataSize > 0)
    {
        if(dataSize < 16)
        {
            *(buffer+(bufferPos++)) = 0x90 + dataSize;
        }
        else
        {
            *(buffer+(bufferPos++)) = 0xdc;
            *(buffer+(bufferPos++)) = 0x00;
            *(buffer+(bufferPos++)) = dataSize;
        }
        for(int i=0;i<dataSize;i++)
            serializeInteger(data[i]);
    }
}

/**
  *  @brief Serializa y escribe en el buffer un arreglo de enteros de 16 bits con signo.
  *  @param data        Arreglo de enteros de 16 bits con signo.
  *  @param dataSize    Tamaño del arreglo (hasta 256 elementos).
  *  @return none
  */
void MsgPackMap::serializeIntegerArray(int16_t data[],uint8_t dataSize)
{
    if(dataSize > 0)
    {
        if(dataSize < 16)
        {
            *(buffer+(bufferPos++)) = 0x90 + dataSize;
        }
        else
        {
            *(buffer+(bufferPos++)) = 0xdc;
            *(buffer+(bufferPos++)) = 0x00;
            *(buffer+(bufferPos++)) = dataSize;
        }
        for(int i=0;i<dataSize;i++)
            serializeInteger(data[i]);
    }
}

/**
  *  @brief Serializa y escribe en el buffer un arreglo de enteros de 32 bits con signo.
  *  @param data        Arreglo de enteros de 32 bits con signo.
  *  @param dataSize    Tamaño del arreglo (hasta 256 elementos).
  *  @return none
  */
void MsgPackMap::serializeIntegerArray(int32_t data[],uint8_t dataSize)
{
    if(dataSize > 0)
    {
        if(dataSize < 16)
        {
            *(buffer+(bufferPos++)) = 0x90 + dataSize;
        }
        else
        {
            *(buffer+(bufferPos++)) = 0xdc;
            *(buffer+(bufferPos++)) = 0x00;
            *(buffer+(bufferPos++)) = dataSize;
        }
        for(int i=0;i<dataSize;i++)
            serializeInteger(data[i]);
    }
}

/*********************************************************************
  *
  *  Métodos auxiliares para búsqueda
  *
  ********************************************************************/


/**
  *  @brief Compara una cadena dada con una cadena serializada dentro de la estructura.
  *  @param pos         Posición inicial de la cadena serializada.
  *  @param keyStr      Cadena de caracteres a comparar.
  *  @return bool       Booleano que indica si las cadenas son iguales o no.
  */
bool MsgPackMap::isEqual(uint8_t pos, const char keyStr[])
{
    uint8_t tmp = strlen(keyStr);
    uint8_t cnt = 0;
    for(int i=0;i<tmp;i++,pos++)
    {
        if(*(buffer+pos) == keyStr[i])
            cnt++;
        else
            break;
    }
    if(cnt == tmp)
        return true;
    else
        return false;
}

/**
  *  @brief Busca el valor de una clave y devuelve la posición inicial de los datos si es que
  *         la clave existe o -1 si no existe la clave.
  *  @param keyStr      Cadena de caracteres con la clave a buscar.
  *  @return int        Entero con la posición de los datos si es que la clave existe, -1 si no
  *                     existe la clave.
  */
int MsgPackMap::getDataPosition(const char keyStr[])
{
    int dataSize = strlen(keyStr);
    uint16_t i=1;
    if(dataSize<32)
    {
        uint8_t charToSearch = 0xa0 + dataSize;
        while(i<bufferSize)
        {
            if(*(buffer+i) == charToSearch)
            {
                if(isEqual(i+1,keyStr))
                    return i+1+dataSize;
                else
                    i = i+1+dataSize;
            }
            else
            {
                i++;
            }
        }
    }
    else
    {
        while(i<bufferSize)
        {
            if(*(buffer+i) == 0xd9)
            {
                if(*(buffer+i+1) == bufferSize)
                {
                    if(isEqual(i+2,keyStr))
                        return i+2+dataSize;
                    else
                        i=i+2+dataSize;
                }
                else
                {
                    i++;
                }
            }
        }
    }
    return -1;
}

/**
  *  @brief Busca una clave y devuelve true si se encuentra en la estructura o false si no se encuentra.
  *  @param keyStr      Cadena de caracteres con la clave a buscar.
  *  @return bool       True si la clave existe, false en caso contrario.
  */
bool MsgPackMap::isKeyAvailable(const char keyStr[])
{
    if(getDataPosition(keyStr) != -1)
        return true;
    return false;
}

/*********************************************************************
  *
  *  Métodos para deserializar los datos (msgpack format -> data)
  *
  ********************************************************************/

/**
  *  @brief Deserializa un dato de tipo float32.
  *  @param pos         Posición inicial del stream de datos.
  *  @return float      Dato deserializado.
  */
float MsgPackMap::deserializeFloat(int pos)
{
    dec.numBytes[3] = *(buffer+pos);
    dec.numBytes[2] = *(buffer+pos+1);
    dec.numBytes[1] = *(buffer+pos+2);
    dec.numBytes[0] = *(buffer+pos+3);
    return dec.num;
}

/**
  *  @brief Deserializa un dato de tipo uint8.
  *  @param pos         Posición inicial del stream de datos.
  *  @return float      Dato deserializado.
  */
uint8_t MsgPackMap::deserializeUnsignedInt8(int pos)
{
    return *(buffer+pos);
}

/**
  *  @brief Deserializa un dato de tipo uint16.
  *  @param pos         Posición inicial del stream de datos.
  *  @return float      Dato deserializado.
  */
uint16_t MsgPackMap::deserializeUnsignedInt16(int pos)
{
    union num
    {
        uint16_t num;
        byte raw[2];
    } tmp;
    tmp.raw[1] = *(buffer+pos);
    tmp.raw[0] = *(buffer+pos+1);
    return tmp.num;
}

/**
  *  @brief Deserializa un dato de tipo uint32.
  *  @param pos         Posición inicial del stream de datos.
  *  @return float      Dato deserializado.
  */
uint32_t MsgPackMap::deserializeUnsignedInt32(int pos)
{
    union num
    {
        uint32_t num;
        byte raw[4];
    } tmp;
    tmp.raw[3] = *(buffer+pos);
    tmp.raw[2] = *(buffer+pos+1);
    tmp.raw[1] = *(buffer+pos+2);
    tmp.raw[0] = *(buffer+pos+3);
    return tmp.num;
}

/**
  *  @brief Deserializa un dato de tipo int8.
  *  @param pos         Posición inicial del stream de datos.
  *  @return float      Dato deserializado.
  */
int8_t MsgPackMap::deserializeInt8(int pos)
{
    return *(buffer+pos);
}

/**
  *  @brief Deserializa un dato de tipo int16.
  *  @param pos         Posición inicial del stream de datos.
  *  @return float      Dato deserializado.
  */
int16_t MsgPackMap::deserializeInt16(int pos)
{
    union num
    {
        int16_t num;
        byte raw[2];
    } tmp;
    tmp.raw[1] = *(buffer+pos);
    tmp.raw[0] = *(buffer+pos+1);
    return tmp.num;
}

/**
  *  @brief Deserializa un dato de tipo int32.
  *  @param pos         Posición inicial del stream de datos.
  *  @return float      Dato deserializado.
  */
int32_t MsgPackMap::deserializeInt32(int pos)
{
    union num
    {
        int32_t num;
        byte raw[4];
    } tmp;
    tmp.raw[3] = *(buffer+pos);
    tmp.raw[2] = *(buffer+pos+1);
    tmp.raw[1] = *(buffer+pos+2);
    tmp.raw[0] = *(buffer+pos+3);
    return tmp.num;
}

/**
  *  @brief Deserializa un dato de tipo string.
  *  @param pos         Posición inicial del stream de datos.
  *  @return float      Dato deserializado.
  */
String MsgPackMap::deserializeString(int pos)
{
    uint8_t dataSize;
    String tmp = "";
    int ini;
    if(*(buffer+pos) >= 0xa0 && *(buffer+pos) <= 0xbf)
    {
        dataSize = 0x1f & *(buffer+pos);
        ini=pos+1;
    }
    else //if(*(buffer+pos) == 0xd9)
    {
        dataSize = *(buffer+pos+1);
        ini=pos+2;
    }
    for(int i = ini;i<ini+dataSize;i++)
        tmp += (char)*(buffer+i);
    return tmp;
}

/**
  *  @brief Deserializa un conjunto de bytes.
  *  @param pos         Posición inicial del stream de datos.
  *  @param buf         Buffer donde se almacenan los datos.
  *  @param bufSize     Tamaño del buffer.
  *  @return none
  */
void MsgPackMap::deserializeByte(int pos, byte buf[], uint8_t bufSize)
{
    uint8_t dataSize = *(buffer+pos);
    uint8_t lim;
    int j;
    if(bufSize < dataSize)
        lim = bufSize;
    else
        lim = dataSize;
    for(int i=pos+1, j=0;i<pos+1+lim;i++,j++)
        buf[j] = *(buffer+i);
}

/**
  *  @brief Deserializa un arreglo de dloat.
  *  @param pos         Posición inicial del stream de datos.
  *  @param buf         Buffer donde se almacenan los datos.
  *  @param bufSize     Tamaño del buffer.
  *  @return none
  */
void MsgPackMap::deserializeFloatArray(int pos, float buf[], uint8_t bufSize)
{
    uint8_t dataSize, ini, lim,j;
    if(*(buffer+pos) >= 0x90 && *(buffer+pos) <= 0x9f)
    {
        dataSize = 0x0f & *(buffer+pos);
        ini = pos+1;
    }
    else
    {
        dataSize = *(buffer+pos+1);
        ini = pos + 2;
    }
    if(bufSize < dataSize)
        lim = bufSize*5;
    else
        lim = dataSize*5;
    for(int i=ini,j=0;i<ini+lim;i=i+5,j++)
    {
        if(*(buffer+i) == 0xca)
            buf[j]=deserializeFloat(i+1);
        else
            buf[j] = 0.0;
    }
}


/*********************************************************************
  *
  *  Métodos para agregar elementos al buffer y a la estructura del Map.
  *  Los elementos se componen de par clave-valor.
  *  La clave (key) debe ser forzozamente un elemento de tipo cadena.
  *
  ********************************************************************/

/**
  *  @brief Inicializa la estructura del mapa.
  *  @return none
  */
void MsgPackMap::beginMap()
{
    numElements = 0;
    startPos = 0;
    bufferPos = 0;
    level = 0;
    startPos = bufferPos++;
}

/**
  *  @brief Decreta el inicio de un submapa. Un elemento del mapa puede tener
  *         como valor un submapa. La cantidad máxima de submapas anidados
  *         (submapas dentro de un mismo submapa) o niveles se define por el
  *         valor de la constante MAX_SUBMAPS.
  *  @param keyStr      Cadena que representa la clave asociada al submapa.
  *  @return none
  */
void MsgPackMap::beginSubMap(const char keyStr[])
{
    static uint16_t tmp;
    if(numElements < 15)
    {
        serializeString(keyStr);
        tmp = bufferPos++;
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        tmp = bufferPos++;
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        tmp = bufferPos++;
        *(buffer+(startPos + 2)) = ++numElements;
    }
    positions[level] = startPos;
    elements[level] = numElements;
    startPos = tmp;
    numElements = 0;
    level++;
}

/**
  *  @brief Decreta el fin de un submapa. Es imperativo llamar a la función para indicar
  *         que los siguientes elementos que se agreguen no pertenecen al submapa
  *  @return none
  */
void MsgPackMap::endSubMap()
{
    level--;
    startPos = positions[level];
    numElements = elements[level];
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es de tipo entero.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Entero sin signo de 8 bits.
  *  @return none
  */
void MsgPackMap::addInteger(const char keyStr[],uint8_t data)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos + 2;
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es de tipo entero.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Entero sin signo de 16 bits.
  *  @return none
  */
void MsgPackMap::addInteger(const char keyStr[],uint16_t data)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es de tipo entero.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Entero sin signo de 32 bits.
  *  @return none
  */
void MsgPackMap::addInteger(const char keyStr[],uint32_t data)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es de tipo entero.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Entero con signo de 8 bits.
  *  @return none
  */
void MsgPackMap::addInteger(const char keyStr[],int8_t data)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es de tipo entero.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Entero con signo de 16 bits.
  *  @return none
  */
void MsgPackMap::addInteger(const char keyStr[],int16_t data)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es de tipo entero.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Entero con signo de 32 bits.
  *  @return none
  */
void MsgPackMap::addInteger(const char keyStr[],int32_t data)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeInteger(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es de un número
  *         de punto flotante de 4 bytes. Si la función se invoca inmediatamente
  *         después de iniciarse un submap, el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Número de punto flotante (4 bytes).
  *  @return none
  */
void MsgPackMap::addFloat(const char keyStr[],float data)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeFloat(data);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeFloat(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeFloat(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es otra cadena de caracteres.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Cadena de caracteres de tamaño máximo 256.
  *  @return none
  */
void MsgPackMap::addString(const char keyStr[],const char data[])
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeString(data);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeString(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeString(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es de tipo bool.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. True o False (bool).
  *  @return none
  */
void MsgPackMap::addBool(const char keyStr[],bool data)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeBool(data);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeBool(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeBool(data);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es null.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @return none
  */
void MsgPackMap::addNull(const char keyStr[])
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeNil();
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeNil();
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeNil();
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es un arreglo de bytes.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Arreglo de bytes.
  *  @param dataSize    Tamaño del arreglo.
  *  @return none
  */
void MsgPackMap::addByte(const char keyStr[],byte data[],uint8_t dataSize)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeByte(data,dataSize);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeByte(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeByte(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es un arreglo de tipo float.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Arreglo de tipo float.
  *  @param dataSize    Tamaño del arreglo.
  *  @return none
  */
void MsgPackMap::addFloatArray(const char keyStr[],float data[],uint8_t dataSize)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeFloatArray(data,dataSize);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeFloatArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeFloatArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es un arreglo de enteros.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Arreglo de enteros de 8 bits sin signo.
  *  @param dataSize    Tamaño del arreglo.
  *  @return none
  */
void MsgPackMap::addIntegerArray(const char keyStr[],uint8_t data[],uint8_t dataSize)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es un arreglo de enteros.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Arreglo de enteros de 16 bits sin signo.
  *  @param dataSize    Tamaño del arreglo.
  *  @return none
  */
void MsgPackMap::addIntegerArray(const char keyStr[],uint16_t data[],uint8_t dataSize)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es un arreglo de enteros.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Arreglo de enteros de 32 bits sin signo.
  *  @param dataSize    Tamaño del arreglo.
  *  @return none
  */
void MsgPackMap::addIntegerArray(const char keyStr[],uint32_t data[],uint8_t dataSize)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es un arreglo de enteros.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Arreglo de enteros de 8 bits con signo.
  *  @param dataSize    Tamaño del arreglo.
  *  @return none
  */
void MsgPackMap::addIntegerArray(const char keyStr[],int8_t data[],uint8_t dataSize)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es un arreglo de enteros.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Arreglo de enteros de 16 bits con signo.
  *  @param dataSize    Tamaño del arreglo.
  *  @return none
  */
void MsgPackMap::addIntegerArray(const char keyStr[],int16_t data[],uint8_t dataSize)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/**
  *  @brief Agrega al map un elemento compuesto de un par clave-valor. La clave
  *         consta de una cadena de caracteres y el valor asociado es un arreglo de enteros.
  *         Si la función se invoca inmediatamente después de iniciarse un submap,
  *         el elemento se agrega a dicho submap.
  *  @param keyStr      Clave. Cadena de caracteres de tamaño máximo 256.
  *  @param data        Valor. Arreglo de enteros de 32 bits con signo.
  *  @param dataSize    Tamaño del arreglo.
  *  @return none
  */
void MsgPackMap::addIntegerArray(const char keyStr[],int32_t data[],uint8_t dataSize)
{
    if(numElements < 15)
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+startPos) = 0x80 | ++numElements;
    }
    else if(numElements == 15)
    {
        rearrageBuffer();
        *(buffer+startPos) = 0xde;
        *(buffer+(startPos + 1)) = 0x00;
        bufferPos = bufferPos+2;
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
    else
    {
        serializeString(keyStr);
        serializeIntegerArray(data,dataSize);
        *(buffer+(startPos + 2)) = ++numElements;
    }
}

/*********************************************************************
  *
  *  Métodos para extraer elementos de la estructura del Map. Los métodos
  *  buscan la clave (key) en la estructura y si existe devuelve el .
  *  contenido.
  *
  ********************************************************************/

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un dato de tipo uint8
  *         lo devuelve, en caso de no serlo, devuelve 0.
  *  @param keyStr      Miembro a buscar(key).
  *  @return uint8_t    Dato deserializado (value).
  */
uint8_t MsgPackMap::readUnsignedInt8(const char keyStr[])
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) >= 0x00 && *(buffer+pos) <= 0x7f) //fixInt
        {
            return deserializeUnsignedInt8(pos);
        }
        else if(*(buffer+pos) == 0xcc) //uint8
        {
            return deserializeUnsignedInt8(pos+1);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un dato de tipo uint16
  *         lo devuelve, en caso de no serlo, devuelve 0.
  *  @param keyStr      Miembro a buscar(key).
  *  @return uint8_t    Dato deserializado (value).
  */
uint16_t MsgPackMap::readUnsignedInt16(const char keyStr[])
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) == 0xcd) //uint16
        {
            return deserializeUnsignedInt16(pos+1);
        }
    }
    return 0;
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un dato de tipo uint32
  *         lo devuelve, en caso de no serlo, devuelve 0.
  *  @param keyStr      Miembro a buscar(key).
  *  @return uint8_t    Dato deserializado (value).
  */
uint32_t MsgPackMap::readUnsignedInt32(const char keyStr[])
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) == 0xce) //uint16
        {
            return deserializeUnsignedInt32(pos+1);
        }
    }
    return 0;
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un dato de tipo int8
  *         lo devuelve, en caso de no serlo, devuelve 0.
  *  @param keyStr      Miembro a buscar(key).
  *  @return uint8_t    Dato deserializado (value).
  */
int8_t MsgPackMap::readInt8(const char keyStr[])
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) >= 0xe0 && *(buffer+pos) <= 0xff) //fixInt
        {
            return deserializeInt8(pos);
        }
        else if(*(buffer+pos) == 0xd0) //uint8
        {
            return deserializeInt8(pos+1);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un dato de tipo int16
  *         lo devuelve, en caso de no serlo, devuelve 0.
  *  @param keyStr      Miembro a buscar(key).
  *  @return uint8_t    Dato deserializado (value).
  */
int16_t MsgPackMap::readInt16(const char keyStr[])
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) == 0xd1) //uint16
        {
            return deserializeInt16(pos+1);
        }
    }
    return 0;
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un dato de tipo int32
  *         lo devuelve, en caso de no serlo, devuelve 0.
  *  @param keyStr      Miembro a buscar(key).
  *  @return uint8_t    Dato deserializado (value).
  */
int32_t MsgPackMap::readInt32(const char keyStr[])
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) == 0xd2) //uint16
        {
            return deserializeInt32(pos+1);
        }
    }
    return 0;
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un dato de tipo float
  *         lo devuelve, en caso de no serlo, devuelve 0.
  *  @param keyStr      Miembro a buscar(key).
  *  @return uint8_t    Dato deserializado (value).
  */
float MsgPackMap::readFloat(const char keyStr[])
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) == 0xca) //uint16
        {
            return deserializeFloat(pos+1);
        }
    }
    return 0.0;
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un dato de tipo bool
  *         lo devuelve, en caso de no serlo, devuelve 0 (false).
  *  @param keyStr      Miembro a buscar(key).
  *  @return uint8_t    Dato deserializado (value).
  */
bool MsgPackMap::readBool(const char keyStr[])
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) == 0xc3)  //bool
        {
            return true;
        }
    }
    return false;
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un dato de tipo String
  *         lo devuelve, en caso de no serlo, devuelve una cadena vacía.
  *  @param keyStr      Miembro a buscar(key).
  *  @return uint8_t    Dato deserializado (value).
  */
String MsgPackMap::readString(const char keyStr[])
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) >= 0xa0 && *(buffer+pos) <= 0xbf || *(buffer+pos) == 0xd9) //fixStr o str8
        {
            return deserializeString(pos);
        }
    }
    return "";
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un conjunto de  bytes
  *         lo devuelve, en caso de no serlo, devuelve 0.
  *  @param keyStr      Miembro a buscar(key).
  *  @param buf         Buffer donde se almacenan los datos.
  *  @param bufSize     Tamaño del buffer.
  *  @return bool       Regresa true si se completo la operación, false
  *                     si no existe el miembro.
  */
bool MsgPackMap::readByte(const char keyStr[], byte buf[], uint8_t bufSize)
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) == 0xc4)
        {
            deserializeByte(pos+1,buf,bufSize);
            return true;
        }
    }
    return false;
}

/**
  *  @brief Busca si la estructura contiene al miembro indicado en keyStr.
  *         En caso de existir, si el contenido es un arreglo de float
  *         lo devuelve, en caso de no serlo, devuelve 0.
  *  @param keyStr      Miembro a buscar(key).
  *  @param buf         Buffer donde se almacenan los datos.
  *  @param bufSize     Tamaño del buffer.
  *  @return bool       Regresa true si se completo la operación, false
  *                     si no existe el miembro.
  */
bool MsgPackMap::readFloatArray(const char keyStr[], float buf[], uint8_t bufSize)
{
    int pos = getDataPosition(keyStr);
    if(pos != -1)
    {
        if(*(buffer+pos) >= 0x90 && *(buffer+pos) <= 0x9f || *(buffer+pos) == 0xdc)
        {
            deserializeFloatArray(pos,buf,bufSize);
            return true;
        }
    }
    return false;
}
