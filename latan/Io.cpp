/*
 * Io.cpp, part of LatAnalyze 3
 *
 * Copyright (C) 2013 - 2014 Antonin Portelli
 *
 * LatAnalyze 3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LatAnalyze 3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LatAnalyze 3.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <latan/Io.hpp>
#include <latan/includes.hpp>

using namespace std;
using namespace Latan;

/******************************************************************************
 *                          File implementation                               *
 ******************************************************************************/
// constructors ////////////////////////////////////////////////////////////////
File::File(void)
: name_("")
, mode_(Mode::null)
, data_()
{}

File::File(const string &name, const unsigned int mode)
: name_(name)
, mode_(mode)
, data_()
{}

// destructor //////////////////////////////////////////////////////////////////
File::~File(void)
{
    deleteData();
}

// access //////////////////////////////////////////////////////////////////////
string File::getName(void) const
{
    return name_;
}

unsigned int File::getMode(void) const
{
    return mode_;
}

// internal functions //////////////////////////////////////////////////////////
void File::deleteData(void)
{
    IoDataTable::iterator i;
    
    for (i=data_.begin();i!=data_.end();++i)
    {
        delete i->second;
    }
    data_.clear();
}

void File::checkWritability(void)
{
    if (!((mode_ & Mode::write)||(mode_ & Mode::append))||!isOpen())
    {
        LATAN_ERROR(Io, "file '" + name_ + "' is not writable");
    }
}

/******************************************************************************
 *                        AsciiFile implementation                            *
 ******************************************************************************/
// AsciiParserState constructor ////////////////////////////////////////////////
AsciiFile::AsciiParserState::AsciiParserState(istream* stream, string* name,
                                              IoDataTable* data)
: ParserState<IoDataTable>(stream, name, data)
{
    initScanner();
}

// AsciiParserState destructor /////////////////////////////////////////////////
AsciiFile::AsciiParserState::~AsciiParserState(void)
{
    destroyScanner();
}

// constructor /////////////////////////////////////////////////////////////////
AsciiFile::AsciiFile(void)
: File(), fileStream_()
, isParsed_(false)
, state_(NULL)
{}

AsciiFile::AsciiFile(const string &name, const unsigned int mode)
{
    open(name, mode);
}

// destructor //////////////////////////////////////////////////////////////////
AsciiFile::~AsciiFile(void)
{
    close();
}

// access //////////////////////////////////////////////////////////////////////
void AsciiFile::save(const DMat &m, const std::string &name)
{
    checkWritability();
    fileStream_ << "#L latan_begin mat " << name << endl;
    fileStream_ << m.cols() << endl;
    fileStream_ << m << endl;
    fileStream_ << "#L latan_end mat " << endl;
}

// tests ///////////////////////////////////////////////////////////////////////
bool AsciiFile::isOpen() const
{
    return fileStream_.is_open();
}

// IO //////////////////////////////////////////////////////////////////////////
void AsciiFile::close(void)
{
    delete state_;
    state_ = NULL;
    if (isOpen())
    {
        fileStream_.close();
    }
    name_     = "";
    mode_     = Mode::null;
    isParsed_ = false;
    deleteData();
}

void AsciiFile::open(const string &name, const unsigned int mode)
{
    if (isOpen())
    {
        LATAN_ERROR(Io, "file already opened with name '" + name_ + "'");
    }
    else
    {
        ios_base::openmode stdMode = 0;
        
        if (mode & Mode::write)
        {
            stdMode |= ios::out|ios::trunc;
        }
        if (mode & Mode::read)
        {
            stdMode |= ios::in;
        }
        if (mode & Mode::append)
        {
            stdMode |= ios::out|ios::app;
        }
        name_     = name;
        mode_     = mode;
        isParsed_ = false;
        fileStream_.open(name_.c_str(), stdMode);
        if (mode_ & Mode::read)
        {
            state_ = new AsciiParserState(&fileStream_, &name_, &data_);
        }
        else
        {
            state_ = NULL;
        }
    }
}

void AsciiFile::load(const string &name __dumb)
{
    if ((mode_ & Mode::read)&&(isOpen()))
    {
        if (!isParsed_)
        {
            parse();
        }
    }
    else
    {
        if (isOpen())
        {
            LATAN_ERROR(Io, "file '" + name_ + "' is not opened in read mode");
        }
        else
        {
            LATAN_ERROR(Io, "file not opened");
        }
    }
}

// parser //////////////////////////////////////////////////////////////////////

// Bison/Flex parser declaration
int _ioAscii_parse(AsciiFile::AsciiParserState* state);

void AsciiFile::parse()
{
    _ioAscii_parse(state_);
    isParsed_ = true;
}
