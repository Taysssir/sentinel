#include "sensors_loader.hpp"
#include "eos/configuration/models.pb.h"
#include "tinyxml.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <QDebug>
namespace eos
{
  namespace eosconfig
  {
    namespace tag
    {
      const char SENSORS[] = "sensors";
      const char SENSOR[] = "sensor";
      const char NAME[] = "name";
      const char TYPE[] = "type";
      const char POSITION[] = "position";
      const char SECOND_POSITION[] = "second_position";
      const char ORIENTATION[] = "orientation";
      const char ROBOT_POINT[] = "robot_point";
      const char X[] = "x";
      const char Y[] = "y";
      const char DISTANCE[] = "distance";
      const char ANGLE[] = "angle";
    }

    namespace
    {
      struct TempXY
      {
        TempXY(int x, int y)
            : x(x)
            , y(y)
        {
        }
        int x;
        int y;
      };
    }

    TiXmlNode *operator>>(TiXmlNode *xml, Sensor &sensor)
    {
      sensor.set_name(xmlhelpers::read(xml, tag::NAME));
      sensor.set_type(xmlhelpers::read_type<Sensor::Type>(xml, tag::TYPE));
      TempXY temp = xmlhelpers::read_tuple<TempXY>(xml, tag::POSITION);
      sensor.mutable_position()->set_x(temp.x);
      sensor.mutable_position()->set_y(temp.y);
      temp = xmlhelpers::read_tuple<TempXY>(xml, tag::SECOND_POSITION);
      sensor.mutable_position_second_element()->set_x(temp.x);
      sensor.mutable_position_second_element()->set_y(temp.y);
      temp = xmlhelpers::read_tuple<TempXY>(xml, tag::ROBOT_POINT);
      sensor.mutable_robot_point()->set_x(temp.x);
      sensor.mutable_robot_point()->set_y(temp.y);
      temp = xmlhelpers::read_tuple<TempXY>(xml, tag::DISTANCE);
      sensor.mutable_distance()->set_min(temp.x);
      sensor.mutable_distance()->set_max(temp.y);
      temp = xmlhelpers::read_tuple<TempXY>(xml, tag::ANGLE);
      sensor.mutable_angle()->set_min(temp.x);
      sensor.mutable_angle()->set_max(temp.y);
      sensor.set_orientation(xmlhelpers::read_int(xml, tag::ORIENTATION));
      return xml;
    }

    BaseResource::Ptr SensorsLoader::load(const RawData &data) const
    {
        bool load_json = true;
        if(load_json)
        {
            return loadJson(data);
        }
        else
        {
            return loadXml(data);
        }
    }


    BaseResource::Ptr SensorsLoader::loadJson(const RawData &data) const
    {
        auto sensors = rsc::alloc<Sensors>();
        // patrols->set_timestamp(xmlhelpers::read_timestamp(doc.RootElement()));
        QJsonDocument jdoc = QJsonDocument::fromJson(QByteArray(data.c_str()));
        if(!jdoc.isNull())
        {
            QJsonObject json = jdoc.object();
            QJsonArray jSensors = json[tag::SENSORS].toArray();
            for(auto jSensor : jSensors)
            {

                Sensor *sensor = sensors->add_sensors();
                sensor->set_name(jSensor.toObject()[tag::NAME].toString().toStdString());
                sensor->set_type(static_cast<Sensor::Type>(0 * jSensor.toObject()[tag::TYPE].toString().toInt()));
               //  qInfo() << " SensorsLoader::loadJson : " << jSensor.toObject()[tag::TYPE] << " value"<<jSensor.toObject()[tag::TYPE] .toString().toInt();
                auto jPosArray = jSensor.toObject()[tag::POSITION].toArray();
                for(auto jPos : jPosArray)
                {
                        Sensor_Point *p = sensor->mutable_position();
                        auto jData = jPos.toObject();
                        p->set_x( int(jData[tag::POSITION].toObject()[tag::X].toString().toDouble()));
                        p->set_y( int(jData[tag::POSITION].toObject()[tag::Y].toString().toDouble()));
                  //
                        Sensor_Point *pp = sensor->mutable_position_second_element();
                        pp->set_x( int(jData[tag::SECOND_POSITION].toObject()[tag::X].toString().toDouble()));
                        pp->set_y( int(jData[tag::SECOND_POSITION].toObject()[tag::Y].toString().toDouble()));
                                  //
                        Sensor_Point *ppp = sensor->mutable_robot_point();
                        ppp->set_x( int(jData[tag::ROBOT_POINT].toObject()[tag::X].toString().toDouble()));
                        ppp->set_y( int(jData[tag::ROBOT_POINT].toObject()[tag::Y].toString().toDouble()));
                }


                   // qInfo() << " PatrolsLoader::loadJson : X " << jData[tag::POSITION].toObject()[tag::X] <<  " value" << jData[tag::POSITION].toObject()[tag::X].toString().toDouble();
                   // qInfo() << " PatrolsLoader::loadJson : Y " << jData[tag::POSITION].toObject()[tag::Y] << " value" << jData[tag::POSITION].toObject()[tag::Y].toString().toDouble() ;

             //   }
            }
        }

        return sensors;
    }

    BaseResource::Ptr SensorsLoader::loadXml(const RawData &data) const
    {
        // load json format file
      TiXmlDocument doc;
      doc.Parse(data.c_str(), 0, TIXML_DEFAULT_ENCODING);

      auto sensors = rsc::alloc<Sensors>();

      sensors->set_timestamp(xmlhelpers::read_timestamp(doc.RootElement()));

      TiXmlNode *sensors_xml = doc.FirstChildElement(tag::SENSORS);
      if (sensors_xml)
      {
        TiXmlNode *sensor_xml = sensors_xml->FirstChildElement(tag::SENSOR);
        while (sensor_xml)
        {
          Sensor *sensor = sensors->mutable_sensors()->Add();
          sensor_xml >> *sensor;
          sensor_xml = sensor_xml->NextSibling();
        }
      }
      return sensors;
    }

    TiXmlNode *operator<<(TiXmlNode *xml, Sensor const &sensor)
    {
      xmlhelpers::dump(xml, tag::NAME, sensor.name());
      xmlhelpers::dump(xml, tag::TYPE, sensor.type());
      xmlhelpers::dump_x_y(xml, tag::POSITION, sensor.position().x(), sensor.position().y());
      xmlhelpers::dump_x_y(xml, tag::SECOND_POSITION, sensor.position_second_element().x(), sensor.position_second_element().y());
      xmlhelpers::dump_x_y(xml, tag::ROBOT_POINT, sensor.robot_point().x(), sensor.robot_point().y());
      xmlhelpers::dump(xml, tag::ORIENTATION, sensor.orientation());
      xmlhelpers::dump_x_y(xml, tag::DISTANCE, sensor.distance().min(), sensor.distance().max());
      xmlhelpers::dump_x_y(xml, tag::ANGLE, sensor.angle().min(), sensor.angle().max());
      return xml;
    }

    bool SensorsLoader::save(RawData &data, BaseResource::Ptr const &rsc) const
    {
        static bool isJson = true;
        if(isJson)
        {
            return saveJson(data,rsc);
        }
        else
        {
            return saveXml(data,rsc);
        }
    }

    bool SensorsLoader::saveJson(RawData &data, BaseResource::Ptr const &rsc) const
    {


        QJsonObject jSensors;
        QJsonArray jSensorArray;
        auto sensors = rsc::as<Sensors>(rsc);
        for (auto &sensor : sensors->sensors())
        {
            QJsonObject jSensor;
            QJsonObject jPos,jPos_sec,jPos_robot;
            QJsonObject jPosition;
            QJsonObject jPosition_second;
            QJsonObject jPosition_robot;

            // ------------ Position ------------
            jPos.insert(tag::X, QJsonValue::fromVariant(QString::number(sensor.position().x())));
            jPos.insert(tag::Y, QJsonValue::fromVariant(QString::number(sensor.position().y())));
            jPosition.insert(tag::POSITION, jPos);
            //jPosition.insert(tag::ORIENTATION, QJsonValue::fromVariant(QString::number(sensor.orientation())));

            // ------------ Position Second_element ------------
            jPos_sec.insert(tag::X, QJsonValue::fromVariant(QString::number(sensor.position_second_element().x())));
            jPos_sec.insert(tag::Y, QJsonValue::fromVariant(QString::number(sensor.position_second_element().y())));
            jPosition_second.insert(tag::POSITION, jPos_sec);
            //jPosition_second.insert(tag::ORIENTATION, QJsonValue::fromVariant(QString::number(sensor.orientation())));

            // ------------ Position Robot_point------------
            jPos_robot.insert(tag::X, QJsonValue::fromVariant(QString::number(sensor.robot_point().x())));
            jPos_robot.insert(tag::Y, QJsonValue::fromVariant(QString::number(sensor.robot_point().y())));
            jPosition_robot.insert(tag::POSITION, jPos_robot);
            //jPosition_robot.insert(tag::ORIENTATION, QJsonValue::fromVariant(QString::number(sensor.orientation())));

            // ------------ SENSOR ------------
            jSensor.insert(tag::NAME, QJsonValue::fromVariant(sensor.name().c_str()));
            jSensor.insert(tag::TYPE,QJsonValue::fromVariant((QString::number(sensor.type()))));
            jSensor.insert(tag::POSITION,jPosition);
            jSensor.insert(tag::SECOND_POSITION,jPosition_second);
            jSensor.insert(tag::ROBOT_POINT,jPosition_robot);

            jSensorArray.push_back(jSensor);
        }
        jSensors.insert(tag::SENSORS,jSensorArray);

        QJsonDocument doc(jSensors);
        QString json_str = doc.toJson();
        // data = json_str;
        //  qDebug() << " " << json_str;
        data = json_str.toStdString();
        return not data.empty();
    }


    bool SensorsLoader::saveXml(RawData &data, BaseResource::Ptr const &rsc) const
    {
      TiXmlDocument doc;
      TiXmlElement *root = xmlhelpers::createDocument(tag::SENSORS, doc);

      auto sensors = rsc::as<Sensors>(rsc);

      xmlhelpers::dump_timestamp(root, sensors->timestamp());

      for (auto const &sensor : sensors->sensors())
      {
        TiXmlElement *xml_sensor = new TiXmlElement(tag::SENSOR);
        xml_sensor << sensor;
        root->LinkEndChild(xml_sensor);
      }

      return xmlhelpers::dump(doc, data);
    }
  }
}
