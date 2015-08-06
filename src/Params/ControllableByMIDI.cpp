

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
    std::cout << "~midiControl" << endl;
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
    if(alreadyDeleting){
        std::cout << "already deleting (all)" << endl;
        return;
    }
    alreadyDeleting = true;
    std::cout << "SET already deleting" << endl;
    if(controllers.size() > 0){
        list<midiControl*>::iterator i;
        std::cout << "controllers to delete: " << controllers.size() << endl;
        for(i=controllers.begin(); i != controllers.end();){
            synth->removeMidiControl(*i);
            std::cout << "one deleted" << endl;
            i = controllers.erase(i);
        }
        controllers.clear();

    }
    std::cout << "controllers left: " << controllers.size() << endl;
    std::cout << std::flush;
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
        cout << "ControllableByMIDI::reassignUIControls: controllers :" << controllers.size() << endl;
    }
}

void ControllableByMIDI::unassignUIControls()
{
    if(alreadyDeleting) return;
    list<midiControl*>::iterator i;
    cout <<  "ControllableByMIDI::unassignUIControls" << endl;
    cout <<  std::flush;
    if(controllers.size() > 0){
        //cout <<  "Removing ui controls (" << controllers.size() << ")" << endl; 
        //cout <<  std::flush;
        
        for(i=controllers.begin(); i != controllers.end();i++){
            /*if((*i)->ui != NULL){
                (*i)->ui->controller = NULL;
            }*/
            (*i)->removeUI();
        }
        std::cout << "size of controllers: " << controllers.size() << endl;
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
    if(alreadyDeleting){
        std::cout << "already deleting" << endl;
        return;
    }
    list<midiControl*>::iterator i;
    std::cout << "controllers to delete: " << controllers.size() << endl;
    for(i=controllers.begin(); i != controllers.end();i++){
        if((*i) == ctrl){
            cout << "Deleting reference " << (*i) << endl;
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
        //cout << "Controller writen " << (*i)->channel << " " << (*i)->ccNbr << endl;
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
            cout << "Error on reading ccNbr or channel (" << ccNbr << ", " << channel  << ")" << endl;
            cpt++;
            xml->exitbranch();
            continue;
        }
        cout << "Controller read (" << cpt << ") " << channel << " " << ccNbr << " (" << this << ", " << par << ")" << endl;
        synth->addMidiControl(ccNbr, channel, min, max, this, NULL, par, false);
        cpt++;
        xml->exitbranch();
    }
    xml->exitbranch();
};