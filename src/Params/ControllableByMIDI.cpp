

using namespace std;
#include "Misc/SynthEngine.h"
#include "Params/ControllableByMIDI.h"
#include "Misc/ControllableByMIDIUI.h"
#include "Misc/XMLwrapper.h"
#include <iostream>
#include <list>

midiControl::midiControl(int ccNbr, int channel, int min, int max, ControllableByMIDI *controller, ControllableByMIDIUI *ui, int par, bool recording): 
        ccNbr(ccNbr), 
        channel(channel), 
        min(min), 
        max(max), 
        controller(controller), 
        ui(ui), 
        par(par), 
        recording(recording) 
{
    init();
}

midiControl::midiControl(ControllableByMIDI *controller, ControllableByMIDIUI *ui, int par ): 
        ccNbr(-1), 
        channel(-1), 
        min(0), max(127), 
        controller(controller), 
        ui(ui), 
        par(par), 
        recording(true) 
{
    init();
}

void midiControl::init()
{
    if(!controller){
        // << "midiControl: the controller should be initialized !" << endl;
    }
    else {
        controller->addMidiController(this);
    }
}

midiControl::~midiControl()
{
    if(controller)
        controller->removeMidiController(this);
}

void midiControl::changepar(int value)
{
    //// << "Par changed, par: " << par << ", value: " << value << endl;
    controller->changepar(par, value);
}

float midiControl::getpar()
{
    if(!controller) return 0;

    return controller->getpar(par);
}

void midiControl::removeUI()
{
    ui = NULL;
}

/*ControllableByMIDI::~ControllableByMIDI(){
    removeAllMidiControllers();
}*/

void ControllableByMIDI::removeAllMidiControllers(SynthEngine *synth)
{
    if(alreadyDeleting) return;
    
    if(controllers.size() > 0){
        list<midiControl*>::iterator i;
        for(i=controllers.begin(); i != controllers.end();){
            synth->removeMidiControl(*i);
            i = controllers.erase(i);
        }
        controllers.clear();

    }
    alreadyDeleting = false;
}

void ControllableByMIDI::reassignUIControls(ControllableByMIDIUI *ctrl)
{
    if(controllers.size()>0){
        list<midiControl*>::iterator i;
        // << "Recreating ui controls (" << controllers.size() << ")" << endl; 
        for(i=controllers.begin(); i != controllers.end();i++){
            (*i)->ui = ctrl;
        }
    }
}

void ControllableByMIDI::unassignUIControls()
{
    if(alreadyDeleting) return;
    list<midiControl*>::iterator i;

    if(controllers.size() > 0){
        for(i=controllers.begin(); i != controllers.end();i++){
            (*i)->removeUI();
        }
    }
}

void ControllableByMIDI::addMidiController(midiControl *ctrl)
{
    list<midiControl*>::iterator i;
    for(i=controllers.begin(); i != controllers.end();i++){
        if((*i) == ctrl || (*i)->par == ctrl->par){
            return;
        }
    }
    controllers.push_back(ctrl);
}

void ControllableByMIDI::removeMidiController(midiControl *ctrl)
{
    if(alreadyDeleting) return;
    
    list<midiControl*>::iterator i;
    for(i=controllers.begin(); i != controllers.end();i++){
        if((*i) == ctrl){
            controllers.erase(i);
            return;
        }
    }
}

midiControl *ControllableByMIDI::hasMidiController(int par)
{
    list<midiControl*>::iterator i;
    for(i=controllers.begin(); i != controllers.end();i++){
        if((*i)->par == par){
            return (*i);
        }
    }
    return NULL;
}

void ControllableByMIDI::add2XMLMidi(XMLwrapper *xml)
{
    if(controllers.size() == 0)
        return;
    xml->beginbranch("MIDI_CONTROLLERS");
    list<midiControl*>::iterator i;
    int cpt = 0;
    for(i = controllers.begin(); i != controllers.end(); i++){
        xml->beginbranch("CONTROLLER", cpt);
        xml->addpar("ccNbr", (*i)->ccNbr);
        xml->addpar("channel", (*i)->channel);
        xml->addpar("min", (*i)->min);
        xml->addpar("max", (*i)->max);
        xml->addpar("par", (*i)->par);
        //xml->addparbool("isFloat", (*i)->isFloat);
        xml->endbranch();
        cpt++;
    }
    xml->endbranch();
};

void ControllableByMIDI::getfromXMLMidi(XMLwrapper *xml, SynthEngine *synth)
{
    if(!xml->enterbranch("MIDI_CONTROLLERS"))
        return;
    int cpt = 0;
    int ccNbr, channel, par, min, max;
    while(xml->enterbranch("CONTROLLER", cpt) != false){
        ccNbr = xml->getpar127("ccNbr", -1);
        channel = xml->getpar127("channel", -1);
        min = xml->getpar127("min", 0);
        max = xml->getpar127("max", 127);
        par = xml->getpar("par", -1, 0, 30);
        xml->getparbool("isFloat", 1); // to be compliant with previous version
        if(ccNbr == -1 || channel == -1){
            //synth->getRuntime().Log("Error on reading ccNbr or channel (" + synth->asString(ccNbr) + ", " + synth->asString(channel) + ")");
            std::cout << "Error on reading ccNbr or channel (" << ccNbr << ", " << channel << ")" << endl;
            cpt++;
            xml->exitbranch();
            continue;
        }
        //synth->getRuntime().Log("Controller read (" + synth->asString(cpt) + ") " + synth->asString(channel) + " " + synth->asString(ccNbr) + " (" + synth->asString((long)this) + ", " + synth->asString(par) + ")");
        std::cout << "Controller read (" << cpt << ") " << channel << " " << ccNbr << " (" << this << ", " << par << ")" << endl;

        synth->addMidiControl(ccNbr, channel, min, max, this, NULL, par, false);
        cpt++;
        xml->exitbranch();
    }
    xml->exitbranch();
};