#include "misc.h"
#define STS_NET_IMPLEMENTATION
#include "sts_net/sts_net.h"
#include "s7/s7.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static sts_net_set_t set;
static sts_net_socket_t server;
static sts_net_socket_t clients[STS_NET_SET_SOCKETS];

static PoolAllocator<Vec2>* vec2_pool = 0;

static void panic(const char* msg)
{
  fprintf(stderr, "PANIC: %s\n\n", msg);
  abort();
}

static void load_script(s7_scheme* sc, const char* name)
{
  s7_pointer res = s7_load(sc, name);
  if (res == s7_nil(sc)) {
    fprintf(stderr, "can't load %s.\n", name);
    abort();
  }
}

static int vec2_type_tag = 0;

static bool is_vec2(s7_pointer o)
{
  return s7_is_c_object(o) && s7_c_object_type(o) == vec2_type_tag;
}

static s7_pointer parse_args(s7_scheme* sc, const char* caller, s7_pointer args, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const char* p = format;
  s7_pointer retval = 0;
  int argi = 1;
  while (*p && retval == 0) {
    s7_pointer arg = s7_car(args);
    args = s7_cdr(args);
    switch (*p) {
    case 'f':
      if (s7_is_number(arg)) {
        f32* tmp = va_arg(va, f32*);
        *tmp = (f32)s7_number_to_real(sc, arg);
      } else {
        retval = s7_wrong_type_arg_error(sc, caller, argi, arg, "number");
      }
      break;
    case 'i':
      if (s7_is_integer(arg)) {
        i32* tmp = va_arg(va, i32*);
        *tmp = (i32)s7_integer(arg);
      } else {
        retval = s7_wrong_type_arg_error(sc, caller, argi, arg, "integer");
      }
      break;
    case 's':
      if (s7_is_string(arg)) {
        const char** tmp = va_arg(va, const char**);
        *tmp = s7_string(arg);
      } else {
        retval = s7_wrong_type_arg_error(sc, caller, argi, arg, "string");
      }
      break;
    case 'v':
      if (is_vec2(arg)) {
        Vec2** tmp = va_arg(va, Vec2**);
        *tmp = (Vec2*)s7_c_object_value(arg);
      } else {
        retval = s7_wrong_type_arg_error(sc, caller, argi, arg, "vec2");
      }
      break;
    default: {
      char buf[128];
      sprintf(buf, "undefined parse_args() token: \'%c\'", *p);
      retval = s7_wrong_type_arg_error(sc, caller, argi, arg, buf);
    } break;
    }
    p++;
    argi++;
  }
  va_end(va);
  return retval;
}

static s7_pointer vec2_to_string(s7_scheme* sc, s7_pointer args)
{
  Vec2* v;
  if (auto err = parse_args(sc, "vec2 to string", args, "v", &v)) {
    return err;
  }
  char buf[256];
  snprintf(buf, sizeof(buf), "<vec2 %.4f %.4f>", v->x, v->y);
  return s7_make_string(sc, buf);
}

static void free_vec2(void* val)
{
  if (val) {
    // printf("freeing Vec2\n");
    vec2_pool->free((Vec2*)val);
  }
}

static bool equal_vec2(void* val1, void* val2)
{
  return val1 == val2;
}

static s7_pointer vec2(s7_scheme* sc, s7_pointer args)
{
  f32 x, y;
  if (auto err = parse_args(sc, "vec2", args, "ff", &x, &y)) {
    return err;
  }
  Vec2* o = vec2_pool->allocate();
  o->x = x;
  o->y = y;
  return s7_make_c_object(sc, vec2_type_tag, (void*)o);
}

static s7_pointer vec2p(s7_scheme* sc, s7_pointer args)
{
  return s7_make_boolean(sc, is_vec2(s7_car(args)));
}

static s7_pointer vec2_x(s7_scheme* sc, s7_pointer args)
{
  Vec2* v;
  if (auto err = parse_args(sc, "vec2-x", args, "v", &v)) {
    return err;
  }
  return s7_make_real(sc, v->x);
}

static s7_pointer set_vec2_x(s7_scheme* sc, s7_pointer args)
{
  Vec2* v;
  f32 f;
  if (auto err = parse_args(sc, "vec2-x", args, "vf", &v, &f)) {
    return err;
  }
  v->x = f;
  return s7_undefined(sc);
}

static s7_pointer vec2_y(s7_scheme* sc, s7_pointer args)
{
  Vec2* v;
  if (auto err = parse_args(sc, "vec2-y", args, "v", &v)) {
    return err;
  }
  return s7_make_real(sc, v->y);
}

static s7_pointer set_vec2_y(s7_scheme* sc, s7_pointer args)
{
  Vec2* v;
  f32 f;
  if (auto err = parse_args(sc, "vec2-y", args, "vf", &v, &f)) {
    return err;
  }
  v->y = f;
  return s7_undefined(sc);
}

static s7_pointer vec2_plus(s7_scheme* sc, s7_pointer args)
{
  Vec2 *a, *b;
  if (auto err = parse_args(sc, "vec2+", args, "vv", &a, &b)) {
    return err;
  }
  Vec2* res = vec2_pool->allocate();
  res->x = a->x + b->x;
  res->y = a->y + b->y;
  return s7_make_c_object(sc, vec2_type_tag, (void*)res);
}

static s7_pointer vec2_minus(s7_scheme* sc, s7_pointer args)
{
  Vec2 *a, *b;
  if (auto err = parse_args(sc, "vec2-", args, "vv", &a, &b)) {
    return err;
  }
  Vec2* res = vec2_pool->allocate();
  res->x = a->x - b->x;
  res->y = a->y - b->y;
  return s7_make_c_object(sc, vec2_type_tag, (void*)res);
}

static s7_pointer vec2_mult(s7_scheme* sc, s7_pointer args)
{
  Vec2* a;
  f32 s;
  if (auto err = parse_args(sc, "vec2*", args, "vf", &a, &s)) {
    return err;
  }
  Vec2* res = vec2_pool->allocate();
  res->x = a->x * s;
  res->y = a->y * s;
  return s7_make_c_object(sc, vec2_type_tag, (void*)res);
}

static s7_pointer vec2_div(s7_scheme* sc, s7_pointer args)
{
  Vec2* a;
  f32 s;
  if (auto err = parse_args(sc, "vec2/", args, "vf", &a, &s)) {
    return err;
  }
  Vec2* res = vec2_pool->allocate();
  res->x = a->x / s;
  res->y = a->y / s;
  return s7_make_c_object(sc, vec2_type_tag, (void*)res);
}

static s7_pointer ease_linear(s7_scheme* sc, s7_pointer args)
{
  f32 t;
  if (auto err = parse_args(sc, "ease-linear", args, "f", &t)) {
    return err;
  }
  return s7_make_real(sc, ::ease_linear(t));
}

static s7_pointer ease_cubic_in(s7_scheme* sc, s7_pointer args)
{
  f32 t;
  if (auto err = parse_args(sc, "ease-cubic-in", args, "f", &t)) {
    return err;
  }
  return s7_make_real(sc, ::ease_cubic_in(t));
}

static s7_pointer ease_cubic_out(s7_scheme* sc, s7_pointer args)
{
  f32 t;
  if (auto err = parse_args(sc, "ease-cubic-out", args, "f", &t)) {
    return err;
  }
  return s7_make_real(sc, ::ease_cubic_out(t));
}

static s7_pointer ease_cubic_in_out(s7_scheme* sc, s7_pointer args)
{
  f32 t;
  if (auto err = parse_args(sc, "ease-cubic-in-out", args, "f", &t)) {
    return err;
  }
  return s7_make_real(sc, ::ease_cubic_in_out(t));
}

static s7_pointer rnd01(s7_scheme* sc, s7_pointer args)
{
  (void)args;
  return s7_make_real(sc, ::rnd01());
}

static s7_pointer rnd(s7_scheme* sc, s7_pointer args)
{
  f32 a, b;
  if (auto err = parse_args(sc, "rnd", args, "ff", &a, &b)) {
    return err;
  }
  if (b < a) {
    std::swap(a, b);
  }
  return s7_make_real(sc, ::rnd01() * (b - a) + a);
}

static s7_scheme* s7 = 0;

static void init_s7()
{
  s7 = s7_init();
  load_script(s7, "write.scm");

  vec2_pool = new PoolAllocator<Vec2>(256);

  vec2_type_tag = s7_make_c_type(s7, "vec2");
  s7_c_type_set_free(s7, vec2_type_tag, free_vec2);
  s7_c_type_set_equal(s7, vec2_type_tag, equal_vec2);
  s7_c_type_set_to_string(s7, vec2_type_tag, vec2_to_string);

  s7_define_function(s7, "vec2", vec2, 2, 0, false, 0);
  s7_define_function(s7, "vec2?", vec2p, 1, 0, false, 0);
  s7_define_variable(s7, "vec2-x",
                     s7_dilambda(s7, "vec2-x", vec2_x, 1, 0, set_vec2_x, 2, 0, 0));
  s7_define_variable(s7, "vec2-y",
                     s7_dilambda(s7, "vec2-y", vec2_y, 1, 0, set_vec2_y, 2, 0, 0));
  s7_define_function(s7, "vec2+", vec2_plus, 2, 0, false, 0);
  s7_define_function(s7, "vec2-", vec2_minus, 2, 0, false, 0);
  s7_define_function(s7, "vec2*", vec2_mult, 2, 0, false, 0);
  s7_define_function(s7, "vec2/", vec2_div, 2, 0, false, 0);

  s7_define_function(s7, "ease-linear", ease_linear, 1, 0, false, 0);
  s7_define_function(s7, "ease-cubic-in", ease_cubic_in, 1, 0, false, 0);
  s7_define_function(s7, "ease-cubic-out", ease_cubic_out, 1, 0, false, 0);
  s7_define_function(s7, "ease-cubic-in-out", ease_cubic_in_out, 1, 0, false, 0);
  s7_define_function(s7, "rnd01", rnd01, 0, 0, false, 0);
  s7_define_function(s7, "rnd", rnd, 2, 0, false, 0);

  load_script(s7, "main.scm");
}

static void listen()
{
  if (sts_net_check_socket_set(&set, 0.0f) < 0) {
    panic(sts_net_get_last_error());
  }
  if (server.ready) {
    for (int i = 0; i < STS_NET_SET_SOCKETS; i++) {
      if (clients[i].fd == INVALID_SOCKET) {
        if (sts_net_accept_socket(&server, &clients[i]) < 0) {
          panic(sts_net_get_last_error());
        }
        if (sts_net_add_socket_to_set(&clients[i], &set) < 0) {
          panic(sts_net_get_last_error());
        }
        puts("client connected.");
        std::string res = "> ";
        if (sts_net_send(&clients[i], res.c_str(), res.size()) < 0) {
          panic(sts_net_get_last_error());
        }
        break;
      }
    }
  }
  for (int i = 0; i < STS_NET_SET_SOCKETS; i++) {
    if (clients[i].ready) {
      i32 bytes = 0;
      char buffer[1024];
      std::string message;
      do {
        bytes = sts_net_recv(&clients[i], buffer, sizeof(buffer));
        if (bytes > 0) {
          message += std::string(buffer, bytes);
          if (sts_net_check_socket_set(&set, 0.0f) < 0) {
            panic(sts_net_get_last_error());
          }
        }
      } while (bytes > 0 && clients[i].ready);
      if (bytes <= 0) {
        if (sts_net_remove_socket_from_set(&clients[i], &set) < 0) {
          panic(sts_net_get_last_error());
        }
        sts_net_close_socket(&clients[i]);
        puts("client disconnected.");
        break;
      }
      if (!message.empty()) {
        std::string res;
        if (message != "\n") {
          s7_int gc_err_loc = -1;
          s7_pointer old_err_port = s7_set_current_error_port(s7, s7_open_output_string(s7));
          if (old_err_port != s7_nil(s7)) {
            gc_err_loc = s7_gc_protect(s7, old_err_port);
          }
          s7_int gc_out_loc = -1;
          s7_pointer old_out_port = s7_set_current_output_port(s7, s7_open_output_string(s7));
          if (old_out_port != s7_nil(s7)) {
            gc_out_loc = s7_gc_protect(s7, old_out_port);
          }

          s7_pointer val = s7_eval_c_string(s7, message.c_str());
          const char* out = s7_get_output_string(s7, s7_current_output_port(s7));
          if ((out) && (*out)) {
            res += out;
          }
          const char* err = s7_get_output_string(s7, s7_current_error_port(s7));
          if ((err) && (*err)) {
            res += err;
          }

          s7_close_output_port(s7, s7_current_error_port(s7));
          s7_set_current_error_port(s7, old_err_port);
          if (gc_err_loc != -1) {
            s7_gc_unprotect_at(s7, gc_err_loc);
          }
          s7_close_output_port(s7, s7_current_output_port(s7));
          s7_set_current_output_port(s7, old_out_port);
          if (gc_out_loc != -1) {
            s7_gc_unprotect_at(s7, gc_out_loc);
          }

          if (res.empty()) {    // no error
            char* tmp = s7_object_to_c_string(s7, val);
            res += tmp;
            free(tmp);
          }
          res += "\n";
        }
        res += "> ";
        if (sts_net_send(&clients[i], res.c_str(), res.size()) < 0) {
          panic(sts_net_get_last_error());
        }
      }
    }
  }
}

int main()
{
  for (int i = 0; i < STS_NET_SET_SOCKETS; i++) {
    clients[i].ready = 0;
    clients[i].fd = INVALID_SOCKET;
  }
  sts_net_init();
  if (sts_net_open_socket(&server, NULL, "5555") < 0) {
    panic(sts_net_get_last_error());
  }
  sts_net_init_socket_set(&set);
  if (sts_net_add_socket_to_set(&server, &set) < 0) {
    panic(sts_net_get_last_error());
  }
  printf("listening on localhost...\n");
  init_s7();

  int frame_counter = 0;

  while (1) {    // window_update()

    listen();

    // setup rendering...

    // call scheme frame-entry (main.scm):
    s7_pointer frame_entry = s7_name_to_value(s7, "frame-entry");
    if (frame_entry == s7_undefined(s7)) {
      fprintf(stderr, "frame-entry function not found.\n");
    } else {
      s7_call(s7, frame_entry, s7_nil(s7));
      // TODO: error port?
    }

    // flush rendering

    //printf("%g fps\n", frame_counter / frame_time);
    frame_counter++;
  }
  sts_net_shutdown();
  free(s7);
  return 0;
}
