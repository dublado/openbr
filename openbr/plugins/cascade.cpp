/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright 2012 The MITRE Corporation                                      *
 *                                                                           *
 * Licensed under the Apache License, Version 2.0 (the "License");           *
 * you may not use this file except in compliance with the License.          *
 * You may obtain a copy of the License at                                   *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 * Unless required by applicable law or agreed to in writing, software       *
 * distributed under the License is distributed on an "AS IS" BASIS,         *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 * See the License for the specific language governing permissions and       *
 * limitations under the License.                                            *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/objdetect/objdetect_c.h>
#include "openbr_internal.h"
#include "openbr/core/opencvutils.h"
#include "openbr/core/resource.h"
#include <stdlib.h>

using namespace cv;
    
struct TrainParams{

public:
    QString data;               // REQUIRED: Filepath to store trained classifier
    QString vec;                // REQUIRED: Filepath to store vector of positive samples, default "vector"
    QString img;                // Filepath to source object image. Either this or info is REQUIRED
    QString info;               // Description file of source images. Either this or img is REQUIRED
    QString bg;                 // REQUIRED: Filepath to background list file
    int num;                    // Number of samples to generate
    int bgcolor;            // Background color supplied image (via img)
    int bgthresh;           // Threshold to determine bgcolor match
    bool inv;                   // Invert colors
    bool randinv;               // Randomly invert colors
    int maxidev;                // Max intensity deviation of foreground pixels
    double maxxangle;           // Maximum rotation angle (X)
    double maxyangle;           // Maximum rotation angle (Y)
    double maxzangle;           // Maximum rotation angle (Z)
    bool show;                  // Show generated samples
    int w;                      // REQUIRED: Sample width
    int h;                      // REQUIRED: Sample height
    int numPos;                 // Number of positive samples
    int numNeg;                 // Number of negative samples
    int numStages;              // Number of stages
    int precalcValBufSize;      // Precalculated val buffer size in Mb
    int precalcIdxBufSize;      // Precalculated index buffer size in Mb
    bool baseFormatSave;        // Save in old format
    QString stageType;          // Stage type (BOOST)
    QString featureType;        // Feature type (HAAR, LBP)
    QString bt;                 // Boosted classifier type (DAB, RAB, LB, GAB)
    double minHitRate;          // Minimal hit rate per stage
    double maxFalseAlarmRate;   // Max false alarm rate per stage
    double weightTrimRate;      // Weight for trimming
    int maxDepth;               // Max weak tree depth
    int maxWeakCount;           // Max weak tree count per stage
    QString mode;               // Haar feature mode (BASIC, CORE, ALL)

    TrainParams(){
        num = -1;
        maxidev = -1;
        maxxangle = -1;
        maxyangle = -1;
        maxzangle = -1;
        w = -1;
        h = -1;
        numPos = -1;
        numNeg = -1;
        numStages = -1;
        precalcValBufSize = -1;
        precalcIdxBufSize = -1;
        minHitRate = -1;
        maxFalseAlarmRate = -1;
        weightTrimRate = -1;
        maxDepth = -1;
        maxWeakCount = -1;
        inv = false;
        randinv = false;
        show = false;
        baseFormatSave = false;
        vec = "vector.vec";
        bgcolor = -1;
        bgthresh = -1;
    }
};

QString buildTrainingArgs(TrainParams params){
    QString args = "";
    if (params.data != "") args += "-data " + params.data + " ";
    else return "";
    if (params.vec != "") args += "-vec " + params.vec + " ";
    else return "";
    if (params.bg != "") args += "-bg " + params.bg + " ";
    else return "";
    if (params.numPos >= 0) args += "-numPos " + QString::number(params.numPos) + " ";
    if (params.numNeg >= 0) args += "-numNeg " + QString::number(params.numNeg) + " ";
    if (params.numStages >= 0) args += "-numStages " + QString::number(params.numStages) + " ";
    if (params.precalcValBufSize >= 0) args += "-precalcValBufSize " + QString::number(params.precalcValBufSize) + " ";
    if (params.precalcIdxBufSize >= 0) args += "-precalcIdxBufSize " + QString::number(params.precalcIdxBufSize) + " ";
    if (params.baseFormatSave) args += "-baseFormatSave ";
    if (params.stageType != "") args += "-stageType " + params.stageType + " ";
    if (params.featureType != "") args += "-featureType " + params.featureType + " ";
    if (params.w >= 0) args += "-w " + QString::number(params.w) + " ";
    else return "";
    if (params.h >= 0) args += "-h " + QString::number(params.h) + " ";
    else return "";
    if (params.bt != "") args += "-bt " + params.bt + " ";
    if (params.minHitRate >= 0) args += "-minHitRate " + QString::number(params.minHitRate) + " ";
    if (params.maxFalseAlarmRate >= 0) args += "-maxFalseAlarmRate " + QString::number(params.maxFalseAlarmRate) + " ";
    if (params.weightTrimRate >= 0) args += "-weightTrimRate " + QString::number(params.weightTrimRate) + " ";
    if (params.maxDepth >= 0) args += "-maxDepth " + QString::number(params.maxDepth) + " ";
    if (params.maxWeakCount >= 0) args += "-maxWeakCount " + QString::number(params.maxWeakCount) + " ";
    if (params.mode != "") args += "-mode " + params.mode + " ";
    return args;
}


QString buildSampleArgs(TrainParams params){
    QString args = "";
    if (params.vec != "") args += "-vec "+params.vec+" ";
    else return "";
    if (params.img != "") args += "-img " + params.img + " ";
    else if (params.info != "") args += "-info " + params.info + " ";
    else return "";
    if (params.bg != "") args += "-bg " + params.bg + " ";
    if (params.num > 0) args += "-num " + QString::number(params.num) + " ";
    if (params.bgcolor >=0 ) args += "-bgcolor " + QString::number(params.bgcolor) + " "; 
    if (params.bgthresh >= 0) args += "-bgthresh " + QString::number(params.bgthresh) + " "; 
    if (params.maxidev >= 0) args += "-maxidev " + QString::number(params.maxidev) + " "; 
    if (params.maxxangle >= 0) args += "-maxxangle " + QString::number(params.maxxangle) + " "; 
    if (params.maxyangle >= 0) args += "-maxyangle " + QString::number(params.maxyangle) + " "; 
    if (params.maxzangle >= 0) args += "-maxzangle " + QString::number(params.maxzangle) + " "; 
    if (params.w >= 0) args += "-w " + QString::number(params.w) + " "; 
    if (params.h >= 0) args += "-h " + QString::number(params.h) + " ";
    if (params.show) args += "-show ";
    if (params.inv) args += "-inv ";
    if (params.randinv) args += "-randinv ";
    return args;
}

void execCommand(QString cmd, QString args){
#ifdef _WIN32
        cmd += ".exe";
#endif
        cmd += " " + args;
        system(cmd.toLocal8Bit().data());
}

void genSamples(TrainParams params, QString argStr = ""){
    QString cmdArgs = buildSampleArgs(params);
    if (argStr != "") cmdArgs += " " + argStr;
    execCommand("opencv_createsamples",cmdArgs);
}


void trainCascade(TrainParams params,QString argStr = ""){
    QString cmdArgs = buildTrainingArgs(params);
    if (argStr != "") cmdArgs += " " + argStr;

    execCommand("opencv_traincascade", cmdArgs);
}

QString rectToString(QRectF r){
    QString out = " " + QString::number(r.x()) + " " + QString::number(r.y()) + " " + QString::number(r.width()) + " "+ QString::number(r.height());
    return out;
}
    
namespace br
{
        
class CascadeResourceMaker : public ResourceMaker<CascadeClassifier>
{
    QString file;

public:
    CascadeResourceMaker(const QString &model)
    {
        file = Globals->sdkPath + "/share/openbr/models/";
        if      (model == "Ear")         file += "haarcascades/haarcascade_ear.xml";
        else if (model == "Eye")         file += "haarcascades/haarcascade_eye_tree_eyeglasses.xml";
        else if (model == "FrontalFace") file += "haarcascades/haarcascade_frontalface_alt2.xml";
        else if (model == "ProfileFace") file += "haarcascades/haarcascade_profileface.xml";
        else{
            // Create temp folder if does not exist
            file = model+QDir::separator()+"cascade.xml";
            QDir dir(model);
            if (!dir.exists())
                if (!QDir::current().mkdir(model)) qFatal("Cannot create model.");
            
            // Make sure file can be created
            QFile pathTest(file);
            if (pathTest.exists()) pathTest.remove();
            
            if (!pathTest.open(QIODevice::WriteOnly | QIODevice::Text)) qFatal("Cannot create model.");
            pathTest.remove();
        }                             
    }

private:
    CascadeClassifier *make() const
    {
        CascadeClassifier *cascade = new CascadeClassifier();
        if (!cascade->load(file.toStdString()))
            qFatal("Failed to load: %s", qPrintable(file));
        return cascade;
    }
};


/*!
 * \ingroup transforms
 * \brief Wraps OpenCV cascade classifier
 * \author Josh Klontz \cite jklontz
 */
class CascadeTransform : public MetaTransform
{
    Q_OBJECT
    Q_PROPERTY(QString model READ get_model WRITE set_model RESET reset_model STORED false)
    Q_PROPERTY(int minSize READ get_minSize WRITE set_minSize RESET reset_minSize STORED false)
    Q_PROPERTY(bool ROCMode READ get_ROCMode WRITE set_ROCMode RESET reset_ROCMode STORED false)
    
    // Training parameters 
    Q_PROPERTY(int numStages READ get_numStages WRITE set_numStages RESET reset_numStages STORED false) 
    Q_PROPERTY(int w READ get_w WRITE set_w RESET reset_w STORED false)
    Q_PROPERTY(int h READ get_h WRITE set_h RESET reset_h STORED false)
    Q_PROPERTY(int numPos READ get_numPos WRITE set_numPos RESET reset_numPos STORED false)     
    Q_PROPERTY(int numNeg READ get_numNeg WRITE set_numNeg RESET reset_numNeg STORED false) 
    Q_PROPERTY(int precalcValBufSize READ get_precalcValBufSize WRITE set_precalcValBufSize RESET reset_precalcValBufSize STORED false) 
    Q_PROPERTY(int precalcIdxBufSize READ get_precalcIdxBufSize WRITE set_precalcIdxBufSize RESET reset_precalcIdxBufSize STORED false) 
    Q_PROPERTY(double minHitRate READ get_minHitRate WRITE set_minHitRate RESET reset_minHitRate STORED false)  
    Q_PROPERTY(double maxFalseAlarmRate READ get_maxFalseAlarmRate WRITE set_maxFalseAlarmRate RESET reset_maxFalseAlarmRate STORED false)  
    Q_PROPERTY(double weightTrimRate READ get_weightTrimRate WRITE set_weightTrimRate RESET reset_weightTrimRate STORED false)  
    Q_PROPERTY(int maxDepth READ get_maxDepth WRITE set_maxDepth RESET reset_maxDepth STORED false) 
    Q_PROPERTY(int maxWeakCount READ get_maxWeakCount WRITE set_maxWeakCount RESET reset_maxWeakCount STORED false) 
    Q_PROPERTY(QString stageType READ get_stageType WRITE set_stageType RESET reset_stageType STORED false) 
    Q_PROPERTY(QString featureType READ get_featureType WRITE set_featureType RESET reset_featureType STORED false) 
    Q_PROPERTY(QString bt READ get_bt WRITE set_bt RESET reset_bt STORED false) 
    Q_PROPERTY(QString mode READ get_mode WRITE set_mode RESET reset_mode STORED false) 
    Q_PROPERTY(bool show READ get_show WRITE set_show RESET reset_show STORED false)    
    Q_PROPERTY(bool baseFormatSave READ get_baseFormatSave WRITE set_baseFormatSave RESET reset_baseFormatSave STORED false)
    Q_PROPERTY(bool overwrite READ get_overwrite WRITE set_overwrite RESET reset_overwrite STORED false)
    
    
    BR_PROPERTY(QString, model, "FrontalFace")
    BR_PROPERTY(int, minSize, 64)
    BR_PROPERTY(bool, ROCMode, false)
        
        // Training parameters - Default values provided trigger OpenCV defaults
    BR_PROPERTY(int, numStages, -1)
    BR_PROPERTY(int,w,-1)
    BR_PROPERTY(int,h,-1)
    BR_PROPERTY(int,numPos,-1)
    BR_PROPERTY(int,numNeg,-1)      
    BR_PROPERTY(int,precalcValBufSize,-1)
    BR_PROPERTY(int,precalcIdxBufSize,-1)
    BR_PROPERTY(double,minHitRate,-1)
    BR_PROPERTY(double,maxFalseAlarmRate,-1)
    BR_PROPERTY(double,weightTrimRate,-1)
    BR_PROPERTY(int,maxDepth,-1)
    BR_PROPERTY(int,maxWeakCount,-1)
    BR_PROPERTY(QString,stageType,"")
    BR_PROPERTY(QString,featureType,"")
    BR_PROPERTY(QString,bt,"")
    BR_PROPERTY(QString,mode,"")
    BR_PROPERTY(bool,show,false)
    BR_PROPERTY(bool,baseFormatSave,false)  
    BR_PROPERTY(bool,overwrite,false)
                                    

    Resource<CascadeClassifier> cascadeResource;

    void init()
    {
        cascadeResource.setResourceMaker(new CascadeResourceMaker(model));
    }
    
    // Train transform
    void train(const TemplateList& data){
        if (overwrite){
            QDir dataDir(model);
            if (dataDir.exists()){
                dataDir.removeRecursively();
                QDir::current().mkdir(model);
            }
        }
        
        FileList files = data.files();
        
        
        // Open positive and negative list files
        QString posFName = "pos.txt";
        QString negFName = "neg.txt";
        QFile posFile(posFName);
        QFile negFile(negFName);
        posFile.open(QIODevice::WriteOnly | QIODevice::Text);
        negFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream posStream(&posFile);
        QTextStream negStream(&negFile);
        
        
        const QString endln = "\r\n";   
        
        int posCount = 0;
        int negCount = 0;
        
        bool buildPos = false; //  If true, build positive vector from single image
        
        TrainParams params;
        
        // Fill in from params (param defaults are same as struct defaults, so no checks are needed)
        params.numStages = numStages;
        params.w = w;
        params.h = h;
        params.numPos = numPos;
        params.numNeg = numNeg;
        params.precalcValBufSize = precalcValBufSize;
        params.precalcIdxBufSize = precalcIdxBufSize;
        params.minHitRate = minHitRate;
        params.maxFalseAlarmRate = maxFalseAlarmRate;
        params.weightTrimRate = weightTrimRate;
        params.maxDepth = maxDepth;
        params.maxWeakCount = maxWeakCount;
        params.stageType = stageType;
        params.featureType = featureType;
        params.bt = bt;
        params.mode = mode;
        params.show = show;
        params.baseFormatSave = baseFormatSave;
        if (params.w < 0)   params.w = minSize;
        if (params.h < 0)   params.h = minSize;
        
        for (int i = 0; i < files.length(); i++){
            File f = files[i];
            if (f.localKeys().contains("training-set")){
                QString tset = f.localMetadata()["training-set"].toString().toLower();
                
                // Negative samples
                if (tset == "neg"){
                    if (negCount > 0) negStream<<endln;
                    negStream << f.path() << QDir::separator() << f.fileName();
                    negCount++;
                
                // Positive samples for crop/rescale    
                }else if (tset == "pos"){
                    
                    if (posCount > 0) posStream<<endln;
                    QString rects = "";
                    
                    // Extract rectangles
                    for (int j = 0; j < f.rects().length(); j++){
                        rects += rectToString(f.rects()[j]);
                        posCount++;
                    }
                    if (f.rects().length() > 0)
                        posStream << f.path() << QDir::separator() << f.fileName() << " " << f.rects().length() << " " << rects;
                    
                // Single positive sample for background removal and overlay on negatives
                }else if (tset == "pos-base"){
                    
                    buildPos = true;
                    params.img = f.path() + QDir::separator() + f.fileName();
                    
                    // Parse settings (unique to this one tag)
                    if (f.localKeys().contains("num")) params.num = f.localMetadata()["num"].toInt();
                    if (f.localKeys().contains("bgcolor")) params.bgcolor = f.localMetadata()["bgcolor"].toInt();
                    if (f.localKeys().contains("bgthresh")) params.bgthresh = f.localMetadata()["bgthresh"].toInt();
                    if (f.localKeys().contains("inv")) params.inv =  f.localMetadata()["inv"].toBool();
                    if (f.localKeys().contains("randinv")) params.randinv =  f.localMetadata()["randinv"].toBool();
                    if (f.localKeys().contains("maxidev")) params.maxidev = f.localMetadata()["maxidev"].toInt();
                    if (f.localKeys().contains("maxxangle")) params.maxxangle = f.localMetadata()["maxxangle"].toDouble();
                    if (f.localKeys().contains("maxyangle")) params.maxyangle = f.localMetadata()["maxyangle"].toDouble();
                    if (f.localKeys().contains("maxzangle")) params.maxzangle = f.localMetadata()["maxzangle"].toDouble();  
                }
            }
        }
        
        // Fill in remaining params conditionally
        posFile.close();
        negFile.close();
        if (buildPos){
            if (params.numPos < 0){
                if (params.num > 0) params.numPos = (int)(params.num*.95);
                else params.numPos = 950;
                posFile.remove();
            }
        }else{
            params.info = posFName;
            if (params.numPos < 0){
                params.numPos = (int)(posCount*.95);
            }
        }
        params.bg = negFName;
        params.data = model;
        if (params.num < 0){
            params.num = posCount;
        }
        if (params.numNeg < 0){
            params.numNeg = negCount*10;
        }
        
        
        genSamples(params);
        trainCascade(params);
        if (posFile.exists()) posFile.remove();
        negFile.remove();
    }
    

    void project(const Template &src, Template &dst) const
    {
        TemplateList temp;
        project(TemplateList() << src, temp);
        if (!temp.isEmpty()) dst = temp.first();
    }

    void project(const TemplateList &src, TemplateList &dst) const
    {
        CascadeClassifier *cascade = cascadeResource.acquire();
        foreach (const Template &t, src) {
            const bool enrollAll = t.file.getBool("enrollAll");

            for (int i=0; i<t.size(); i++) {
                const Mat &m = t[i];
                std::vector<Rect> rects;
                std::vector<int> rejectLevels;
                std::vector<double> levelWeights;
                if (ROCMode) cascade->detectMultiScale(m, rects, rejectLevels, levelWeights, 1.2, 5, (enrollAll ? 0 : CASCADE_FIND_BIGGEST_OBJECT) | CASCADE_SCALE_IMAGE, Size(minSize, minSize), Size(), true);
                else         cascade->detectMultiScale(m, rects, 1.2, 5, enrollAll ? 0 : CASCADE_FIND_BIGGEST_OBJECT, Size(minSize, minSize));

                if (!enrollAll && rects.empty())
                    rects.push_back(Rect(0, 0, m.cols, m.rows));

                for (size_t j=0; j<rects.size(); j++) {
                    Template u(t.file, m);
                    if (rejectLevels.size() > j)
                        u.file.set("Confidence", rejectLevels[j]*levelWeights[j]);
                    else 
                        u.file.set("Confidence", 1);
                    const QRectF rect = OpenCVUtils::fromRect(rects[j]);
                    u.file.appendRect(rect);
                    u.file.set(model, rect);
                    dst.append(u);
                }
            }
        }

        cascadeResource.release(cascade);
    }

    // TODO: Remove this code when ready to break binary compatibility
    void store(QDataStream &stream) const
    {
        int size = 1;
        stream << size;
    }

    void load(QDataStream &stream)
    {
        int size;
        stream >> size;
    }
};

BR_REGISTER(Transform, CascadeTransform)

} // namespace br

#include "cascade.moc"
