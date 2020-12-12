#include "TFT_eSPI.h"

TFT_eSPI tft;

enum directions { NONE, NOR = 1, EAS = 2, SOU = 4, WES = 8 };
const int CELL_SIZE = 8;

class MazeGenerator
{
  public:
    MazeGenerator() {
      _world = 0;
    }

    ~MazeGenerator() {
      killArray();      
    }

    void create( int side ) {
      _s = side;
      generate();
      int ox = (320 - (_s * CELL_SIZE)) / 2;
      int oy = (240 - (_s * CELL_SIZE)) / 2;
      display(ox, oy); // at offset
    }

  private:
    void generate() {
      killArray();
      _world = new byte[_s * _s];
      memset(_world, _s * _s, 0);
      _ptX = rand() % _s; _ptY = rand() % _s;
      carve();
    }

    void carve() {
      while ( true ) {
        int d = getDirection();
        if ( d < NOR ) return;

        switch ( d ){
          case NOR:
            _world[_ptX + _s * _ptY] |= NOR; _ptY--;
            _world[_ptX + _s * _ptY] = SOU | SOU << 4;
            break;
          case EAS:
            _world[_ptX + _s * _ptY] |= EAS; _ptX++;
            _world[_ptX + _s * _ptY] = WES | WES << 4;
            break;
          case SOU:
            _world[_ptX + _s * _ptY] |= SOU; _ptY++;
            _world[_ptX + _s * _ptY] = NOR | NOR << 4;
            break;
          case WES:
            _world[_ptX + _s * _ptY] |= WES; _ptX--;
            _world[_ptX + _s * _ptY] = EAS | EAS << 4;
        }
      }
    }

    void display(int ox, int oy) {
      for ( int y = 0; y < _s; y++ )
      {
        int yy = y * _s;
        for ( int x = 0; x < _s; x++ )
        {
          byte b = _world[x + yy];
          int nx = x * CELL_SIZE + ox,
              ny = y * CELL_SIZE + oy;

          if ( !( b & NOR ) ) {
            tft.drawLine(nx, ny, nx + CELL_SIZE + 1, ny, TFT_WHITE);
          }
          if ( !( b & EAS ) ) {
            tft.drawLine( nx + CELL_SIZE, ny, nx + CELL_SIZE, ny + CELL_SIZE + 1, TFT_WHITE);
          }
          if ( !( b & SOU ) ) {
            tft.drawLine(nx, ny + CELL_SIZE, nx + CELL_SIZE + 1, ny + CELL_SIZE, TFT_WHITE);
          }
          if ( !( b & WES ) ) {
            tft.drawLine(nx, ny, nx, ny + CELL_SIZE + 1, TFT_WHITE);
          }
        }
      }      
      
    }

    int getDirection() {
      int d = 1 << rand() % 4;
      while ( true ) {
        for ( int x = 0; x < 4; x++ ) {
          if ( testDir( d ) ) return d;
          d <<= 1;
          if ( d > 8 ) d = 1;
        }
        d = ( _world[_ptX + _s * _ptY] & 0xf0 ) >> 4;
        if ( !d ) return -1;
        switch ( d ) {
          case NOR: _ptY--; break;
          case EAS: _ptX++; break;
          case SOU: _ptY++; break;
          case WES: _ptX--; break;
        }
        d = 1 << rand() % 4;
      }
    }

    bool testDir( int d ) {
      switch ( d ) {
        case NOR: return ( _ptY - 1 > -1 && !_world[_ptX + _s * ( _ptY - 1 )] );
        case EAS: return ( _ptX + 1 < _s && !_world[_ptX + 1 + _s * _ptY] );
        case SOU: return ( _ptY + 1 < _s && !_world[_ptX + _s * ( _ptY + 1 )] );
        case WES: return ( _ptX - 1 > -1 && !_world[_ptX - 1 + _s * _ptY] );
      }
      return false;
    }

    void killArray() {
      if ( _world ) delete [] _world;
    }

    byte*    _world;
    int      _s, _ptX, _ptY;
};

void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  MazeGenerator mg;
  // Maze is square. Max value will be 240/CELL_SIZE = 29
  mg.create(29);
}

void loop() {
  delay(1);
}
